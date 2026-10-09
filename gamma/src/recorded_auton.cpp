#include "recorded_auton.hpp"
#include "autonomous.hpp"
#include "bot_connections.hpp"
#include "controls.hpp"
#include "onboard_gui.hpp"
#include "properties.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>

namespace RecordedAuton {
	namespace {
		constexpr char RECORDING_PREFIX[] = "recording_";
		constexpr char RECORDING_EXTENSION[] = ".txt";
		constexpr char RECORDING_HEADER[] = "RECORDED_AUTON_V1";
		constexpr float MAX_STEERING_RATIO = 0.5f;
		constexpr float MIN_FORWARD_DELTA_CDEG = 500.0f;
		constexpr std::size_t PATH_SEARCH_WINDOW = 300;
		constexpr std::uint32_t MAX_REPLAY_OVERRUN_MSEC = 10000;

		struct Sample {
			std::uint32_t time_msec;
			std::int32_t heading_cdeg;
			std::int32_t lr_delta_cdeg;
			std::int32_t fb_delta_cdeg;
			std::uint16_t buttons;
			std::int16_t forward_input;
		};

		struct Point {
			float x;
			float y;
			float heading;
			Sample sample;
		};

		std::ofstream recording_stream;
		std::string recording_filename;
		std::uint32_t recording_time_msec = 0;
		std::int32_t last_lr_position = 0;
		std::int32_t last_fb_position = 0;
		std::uint32_t samples_since_flush = 0;
		bool previous_trigger_state = false;

		bool pressed(pros::Controller& controller, pros::controller_digital_e_t key) {
			return controller.get_digital(key);
		}

		std::uint16_t read_buttons(pros::Controller& controller) {
			std::uint16_t mask = 0;
			if (pressed(controller, pros::E_CONTROLLER_DIGITAL_L1)) mask |= BUTTON_L1;
			if (pressed(controller, pros::E_CONTROLLER_DIGITAL_L2)) mask |= BUTTON_L2;
			if (pressed(controller, pros::E_CONTROLLER_DIGITAL_R1)) mask |= BUTTON_R1;
			if (pressed(controller, pros::E_CONTROLLER_DIGITAL_R2)) mask |= BUTTON_R2;
			if (pressed(controller, pros::E_CONTROLLER_DIGITAL_UP)) mask |= BUTTON_UP;
			if (pressed(controller, pros::E_CONTROLLER_DIGITAL_DOWN)) mask |= BUTTON_DOWN;
			if (pressed(controller, pros::E_CONTROLLER_DIGITAL_LEFT)) mask |= BUTTON_LEFT;
			if (pressed(controller, pros::E_CONTROLLER_DIGITAL_RIGHT)) mask |= BUTTON_RIGHT;
			if (pressed(controller, pros::E_CONTROLLER_DIGITAL_X)) mask |= BUTTON_X;
			if (pressed(controller, pros::E_CONTROLLER_DIGITAL_B)) mask |= BUTTON_B;
			if (pressed(controller, pros::E_CONTROLLER_DIGITAL_Y)) mask |= BUTTON_Y;
			if (pressed(controller, pros::E_CONTROLLER_DIGITAL_A)) mask |= BUTTON_A;
			return mask;
		}

		bool is_complete_recording(const std::string& filename) {
			std::ifstream file("/" + filename);
			std::string line;
			if (!std::getline(file, line) || line != RECORDING_HEADER) return false;

			std::string last_line;
			while (std::getline(file, line)) {
				if (!line.empty()) last_line = line;
			}
			return last_line.compare(0, 4, "END ") == 0;
		}

		bool parse_recording_id(const std::string& filename, std::uint32_t& id) {
			const std::string prefix = RECORDING_PREFIX;
			const std::string extension = RECORDING_EXTENSION;
			if (filename.compare(0, prefix.size(), prefix) != 0
				|| filename.size() <= prefix.size() + extension.size()
				|| filename.compare(filename.size() - extension.size(), extension.size(), extension) != 0) {
				return false;
			}

			const std::string digits = filename.substr(
				prefix.size(),
				filename.size() - prefix.size() - extension.size()
			);
			if (digits.empty()) return false;

			char* end = nullptr;
			const unsigned long parsed = std::strtoul(digits.c_str(), &end, 10);
			if (end == digits.c_str() || *end != '\0' || parsed > 0xffffffffUL) return false;
			id = static_cast<std::uint32_t>(parsed);
			return true;
		}

		bool start_recording() {
			pros::Controller& controller = BotConnections::controller;
			if (!pros::usd::is_installed()) {
				std::printf("Recording failed: SD card is not installed.\n");
				controller.rumble("---");
				return false;
			}

			char files[8192] = {};
			if (pros::usd::list_files("\\", files, sizeof(files)) == PROS_ERR) {
				std::printf("Recording failed: could not list SD card files.\n");
				controller.rumble("---");
				return false;
			}

			std::uint32_t next_id = 1;
			std::istringstream file_list(files);
			for (std::string file; std::getline(file_list, file); ) {
				if (!file.empty() && file.back() == '\r') file.pop_back();
				std::uint32_t id = 0;
				if (parse_recording_id(file, id) && id >= next_id) {
					next_id = id + 1;
				}
			}

			char filename[40];
			std::snprintf(filename, sizeof(filename), "recording_%08lu.txt",
				static_cast<unsigned long>(next_id));
			recording_filename = filename;
			recording_stream.open("/" + recording_filename, std::ios::out | std::ios::trunc);
			if (!recording_stream.is_open()) {
				std::printf("Recording failed: could not create %s on the SD card.\n",
					recording_filename.c_str());
				recording_filename.clear();
				controller.rumble("---");
				return false;
			}

			recording_stream << RECORDING_HEADER << '\n';
			recording_time_msec = 0;
			samples_since_flush = 0;
			last_lr_position = BotConnections::LRODOM.get_position();
			last_fb_position = BotConnections::FBODOM.get_position();

			Sample initial {
				0,
				static_cast<std::int32_t>(std::lround(BotConnections::imu.get_heading() * 100.0)),
				0,
				0,
				static_cast<std::uint16_t>(read_buttons(controller) & ~(BUTTON_DOWN | BUTTON_LEFT)),
				static_cast<std::int16_t>(controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y))
			};
			recording_stream
				<< initial.time_msec << ' '
				<< initial.heading_cdeg << ' '
				<< initial.lr_delta_cdeg << ' '
				<< initial.fb_delta_cdeg << ' '
				<< initial.buttons << ' '
				<< initial.forward_input << '\n';
			recording_stream.flush();

			if (!recording_stream.good()) {
				std::printf("Recording failed while writing its header.\n");
				recording_stream.close();
				recording_filename.clear();
				controller.rumble("---");
				return false;
			}
			std::printf("Recording started: %s\n", recording_filename.c_str());
			controller.rumble(".");
			return true;
		}

		void write_sample(const Sample& sample) {
			recording_stream
				<< sample.time_msec << ' '
				<< sample.heading_cdeg << ' '
				<< sample.lr_delta_cdeg << ' '
				<< sample.fb_delta_cdeg << ' '
				<< sample.buttons << ' '
				<< sample.forward_input << '\n';

			if (++samples_since_flush >= 10) {
				recording_stream.flush();
				samples_since_flush = 0;
				if (!recording_stream.good()) {
					std::printf("Recording stopped: SD card write failed.\n");
					recording_stream.close();
					recording_filename.clear();
					BotConnections::controller.rumble("---");
				}
			}
		}

		float normalize_radians(float angle) {
			while (angle > static_cast<float>(M_PI)) angle -= 2.0f * static_cast<float>(M_PI);
			while (angle < -static_cast<float>(M_PI)) angle += 2.0f * static_cast<float>(M_PI);
			return angle;
		}

		bool load_samples(const std::string& filename, std::vector<Sample>& samples) {
			std::ifstream file("/" + filename);
			std::string line;
			if (!std::getline(file, line) || line != RECORDING_HEADER) return false;

			bool found_end = false;
			while (std::getline(file, line)) {
				if (line.empty()) continue;
				if (line.compare(0, 4, "END ") == 0) {
					found_end = true;
					break;
				}

				std::istringstream row(line);
				Sample sample {};
				unsigned int buttons = 0;
				int forward_input = 0;
				if (!(row >> sample.time_msec
					>> sample.heading_cdeg
					>> sample.lr_delta_cdeg
					>> sample.fb_delta_cdeg
					>> buttons
					>> forward_input)
					|| buttons > 0xffff
					|| forward_input < -127
					|| forward_input > 127) {
					return false;
				}
				sample.buttons = static_cast<std::uint16_t>(buttons);
				sample.forward_input = static_cast<std::int16_t>(forward_input);
				samples.push_back(sample);
			}
			return found_end && !samples.empty();
		}

		std::vector<Point> build_path(const std::vector<Sample>& samples) {
			std::vector<Point> path;
			path.reserve(samples.size());
			const float starting_heading = samples.front().heading_cdeg * static_cast<float>(M_PI) / 18000.0f;
			float x = 0.0f;
			float y = 0.0f;
			float previous_heading = starting_heading;

			for (std::size_t i = 0; i < samples.size(); ++i) {
				const Sample& sample = samples[i];
				const float heading = sample.heading_cdeg * static_cast<float>(M_PI) / 18000.0f;
				const float relative_heading = normalize_radians(heading - starting_heading);
				if (i > 0) {
					const float midpoint_heading = normalize_radians(
						(previous_heading + normalize_radians(heading - previous_heading) / 2.0f)
						- starting_heading
					);
					const float lr = sample.lr_delta_cdeg * Properties::LR_ODOM_DIRECTION;
					const float fb = sample.fb_delta_cdeg * Properties::FB_ODOM_DIRECTION;
					x += lr * std::cos(midpoint_heading) - fb * std::sin(midpoint_heading);
					y += lr * std::sin(midpoint_heading) + fb * std::cos(midpoint_heading);
				}
				path.push_back(Point { x, y, relative_heading, sample });
				previous_heading = heading;
			}
			return path;
		}

		int clamp_motor(int value) {
			return std::max(-127, std::min(value, 127));
		}

		void drive_pure_pursuit(float forward, float turn) {
			// Replay commands are recorded motor inputs, not wheel-speed requests.
			float left = (forward + turn) * Properties::LEFT_DRIVE_BIAS;
			float right = (forward - turn) * Properties::RIGHT_DRIVE_BIAS;

			const float peak_output = std::max(std::abs(left), std::abs(right));
			if (peak_output > 127.0f) {
				const float scale = 127.0f / peak_output;
				left *= scale;
				right *= scale;
			}

			BotConnections::left_mg.move(clamp_motor(static_cast<int>(std::lround(left))));
			BotConnections::right_mg.move(clamp_motor(static_cast<int>(std::lround(right))));
		}
	}

	std::vector<RecordingInfo> list_recordings() {
		std::vector<RecordingInfo> recordings;
		char files[8192] = {};
		if (!pros::usd::is_installed()) return recordings;
		if (pros::usd::list_files("\\", files, sizeof(files)) == PROS_ERR) {
			std::printf("Could not list recordings on the SD card.\n");
			return recordings;
		}

		std::istringstream file_list(files);
		for (std::string filename; std::getline(file_list, filename); ) {
			if (!filename.empty() && filename.back() == '\r') filename.pop_back();
			std::uint32_t id = 0;
			if (!parse_recording_id(filename, id) || !is_complete_recording(filename)) continue;

			char label[48];
			std::snprintf(label, sizeof(label), "Recorded Auton %lu",
				static_cast<unsigned long>(id));
			recordings.push_back(RecordingInfo { filename, label });
		}

		std::sort(recordings.begin(), recordings.end(),
			[](const RecordingInfo& a, const RecordingInfo& b) {
				return a.filename < b.filename;
			});
		return recordings;
	}

	bool is_recording() {
		return recording_stream.is_open();
	}

	void update_driver_recording() {
		pros::Controller& controller = BotConnections::controller;
		const bool trigger_held =
			pressed(controller, pros::E_CONTROLLER_DIGITAL_DOWN)
			&& pressed(controller, pros::E_CONTROLLER_DIGITAL_LEFT);

		if (trigger_held && !previous_trigger_state) {
			if (is_recording()) {
				stop_recording();
			} else {
				start_recording();
			}
		}
		previous_trigger_state = trigger_held;

		if (!is_recording()) return;

		const std::int32_t lr_position = BotConnections::LRODOM.get_position();
		const std::int32_t fb_position = BotConnections::FBODOM.get_position();
		recording_time_msec += Properties::TICK_DELAY_MSEC;
		Sample sample {
			recording_time_msec,
			static_cast<std::int32_t>(std::lround(BotConnections::imu.get_heading() * 100.0)),
			lr_position - last_lr_position,
			fb_position - last_fb_position,
			read_buttons(controller),
			static_cast<std::int16_t>(controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y))
		};

		if (trigger_held) {
			sample.buttons &= static_cast<std::uint16_t>(~(BUTTON_DOWN | BUTTON_LEFT));
		}
		write_sample(sample);
		last_lr_position = lr_position;
		last_fb_position = fb_position;
	}

	void stop_recording() {
		if (!is_recording()) return;

		recording_stream << "END " << recording_time_msec << '\n';
		recording_stream.flush();
		recording_stream.close();
		const bool write_succeeded = recording_stream.good();
		if (!write_succeeded) {
			std::printf("Recording could not be finalized on the SD card.\n");
			recording_filename.clear();
			BotConnections::controller.rumble("---");
			return;
		}

		const std::string completed_filename = recording_filename;
		recording_filename.clear();
		std::printf("Recording saved: %s\n", completed_filename.c_str());
		BotConnections::controller.rumble("..");
		OnboardGUI::refresh_recordings();
	}

	bool restore_selected_recording(const std::string& filename) {
		for (const RecordingInfo& recording : list_recordings()) {
			if (recording.filename == filename) {
				selected_recording_file = filename;
				return true;
			}
		}
		selected_recording_file.clear();
		return false;
	}

	bool select_recording(const std::string& filename) {
		if (!restore_selected_recording(filename)) return false;
		Autonomous::active_routine = nullptr;
		Autonomous::write_recording_cache(filename);
		return true;
	}

	void clear_selected_recording() {
		selected_recording_file.clear();
	}

	void replay_selected_recording() {
		BotConnections::left_mg.brake();
		BotConnections::right_mg.brake();
		std::vector<Sample> samples;
		if (!load_samples(selected_recording_file, samples)) {
			std::printf("Selected recording is missing, incomplete, or invalid: %s\n",
				selected_recording_file.c_str());
			BotConnections::left_mg.brake();
			BotConnections::right_mg.brake();
			return;
		}

		const std::vector<Point> path = build_path(samples);
		if (path.size() < 2) {
			std::printf("Selected recording has no usable path: %s\n",
				selected_recording_file.c_str());
			return;
		}

		while (BotConnections::imu.is_calibrating()
			&& pros::competition::is_autonomous()
			&& !pros::competition::is_disabled()) {
			pros::delay(10);
		}
		if (!pros::competition::is_autonomous() || pros::competition::is_disabled()) return;
		Controls::processLondonButtons(0);
		Controls::processPneumaticsButtons(0);

		const float start_heading = static_cast<float>(BotConnections::imu.get_heading())
			* static_cast<float>(M_PI) / 180.0f;
		float live_x = 0.0f;
		float live_y = 0.0f;
		float previous_heading = start_heading;
		std::int32_t previous_lr = BotConnections::LRODOM.get_position();
		std::int32_t previous_fb = BotConnections::FBODOM.get_position();
		std::size_t progress_index = 0;
		std::size_t sample_index = 0;
		const std::uint32_t start_time = pros::millis();
		const std::uint32_t final_time = samples.back().time_msec;
		int endpoint_speed_limit = 0;
		for (std::vector<Sample>::const_reverse_iterator sample = samples.rbegin();
			sample != samples.rend(); ++sample) {
			if (sample->forward_input != 0) {
				endpoint_speed_limit = std::abs(sample->forward_input);
				break;
			}
		}
		if (endpoint_speed_limit == 0) endpoint_speed_limit = 40;

		while (pros::competition::is_autonomous() && !pros::competition::is_disabled()) {
			const std::uint32_t elapsed = pros::millis() - start_time;
			const std::int32_t lr_position = BotConnections::LRODOM.get_position();
			const std::int32_t fb_position = BotConnections::FBODOM.get_position();
			const float heading = static_cast<float>(BotConnections::imu.get_heading())
				* static_cast<float>(M_PI) / 180.0f;
			const float midpoint_heading = normalize_radians(
				previous_heading + normalize_radians(heading - previous_heading) / 2.0f
			);
			const float lr_delta = (lr_position - previous_lr) * Properties::LR_ODOM_DIRECTION;
			const float fb_delta = (fb_position - previous_fb) * Properties::FB_ODOM_DIRECTION;
			live_x += lr_delta * std::cos(midpoint_heading) - fb_delta * std::sin(midpoint_heading);
			live_y += lr_delta * std::sin(midpoint_heading) + fb_delta * std::cos(midpoint_heading);
			previous_lr = lr_position;
			previous_fb = fb_position;
			previous_heading = heading;
			const float relative_heading = normalize_radians(heading - start_heading);

			while (sample_index + 1 < samples.size()
				&& samples[sample_index + 1].time_msec <= elapsed) {
				++sample_index;
			}
			const std::uint16_t buttons = elapsed <= final_time
				? samples[sample_index].buttons
				: 0;
			Controls::processLondonButtons(buttons);
			Controls::processIntakeButtons(buttons);
			Controls::processPneumaticsButtons(buttons);

			std::size_t nearest_index = progress_index;
			float nearest_distance_squared = INFINITY;
			const std::size_t search_end = std::min(
				progress_index + PATH_SEARCH_WINDOW, path.size() - 1
			);
			for (std::size_t i = progress_index; i <= search_end; ++i) {
				const float dx = path[i].x - live_x;
				const float dy = path[i].y - live_y;
				const float distance_squared = dx * dx + dy * dy;
				if (distance_squared < nearest_distance_squared) {
					nearest_distance_squared = distance_squared;
					nearest_index = i;
				}
			}
			progress_index = nearest_index;

			std::size_t target_index = std::min(
				nearest_index + PATH_SEARCH_WINDOW, path.size() - 1
			);
			for (std::size_t i = nearest_index; i <= target_index; ++i) {
				const float dx = path[i].x - live_x;
				const float dy = path[i].y - live_y;
				if (dx * dx + dy * dy >= Properties::RECORDED_AUTON_LOOKAHEAD_CDEG
					* Properties::RECORDED_AUTON_LOOKAHEAD_CDEG) {
					target_index = i;
					break;
				}
			}

			const float target_dx = path[target_index].x - live_x;
			const float target_dy = path[target_index].y - live_y;
			const float local_left = target_dx * std::cos(relative_heading)
				+ target_dy * std::sin(relative_heading);
			const float target_distance_squared = target_dx * target_dx + target_dy * target_dy;

			float forward = elapsed <= final_time
				? static_cast<float>(samples[sample_index].forward_input)
				: 0.0f;
			if (elapsed > final_time) {
				const Point& endpoint = path.back();
				const float endpoint_dx = endpoint.x - live_x;
				const float endpoint_dy = endpoint.y - live_y;
				const float endpoint_distance = std::sqrt(
					endpoint_dx * endpoint_dx + endpoint_dy * endpoint_dy
				);
				const float endpoint_forward = -endpoint_dx * std::sin(relative_heading)
					+ endpoint_dy * std::cos(relative_heading);
				const float approach_speed = std::min(
					static_cast<float>(endpoint_speed_limit),
					std::max(20.0f,
						endpoint_distance / Properties::RECORDED_AUTON_LOOKAHEAD_CDEG
							* endpoint_speed_limit)
				);
				forward = (endpoint_forward < 0.0f ? -1.0f : 1.0f) * approach_speed;
			}
			float steering_ratio = target_distance_squared > 1.0f
				? local_left / std::sqrt(target_distance_squared)
				: 0.0f;
			steering_ratio = std::max(
				-MAX_STEERING_RATIO,
				std::min(steering_ratio, MAX_STEERING_RATIO)
			);
			const float heading_error = normalize_radians(
				path[sample_index].heading - relative_heading
			);
			const float heading_correction = std::max(
				-Properties::RECORDED_AUTON_MAX_HEADING_CORRECTION,
				std::min(
					Properties::RECORDED_AUTON_HEADING_GAIN * heading_error,
					Properties::RECORDED_AUTON_MAX_HEADING_CORRECTION
				)
			);
			const float turn = -forward * steering_ratio + heading_correction;

			if (nearest_index + 1 >= path.size()
				&& nearest_distance_squared <= MIN_FORWARD_DELTA_CDEG * MIN_FORWARD_DELTA_CDEG
				&& elapsed > final_time) {
				break;
			}
			if (elapsed > final_time + MAX_REPLAY_OVERRUN_MSEC) {
				std::printf("Replay ended at its maxzm overrun time.\n");
				break;
			}

			drive_pure_pursuit(forward, turn);
			pros::delay(Properties::TICK_DELAY_MSEC);
		}

		BotConnections::left_mg.brake();
		BotConnections::right_mg.brake();
		Controls::processLondonButtons(0);
		Controls::processIntakeButtons(0);
		Controls::processPneumaticsButtons(0);
	}
}
