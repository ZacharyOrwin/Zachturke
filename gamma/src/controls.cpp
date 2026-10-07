#include "controls.hpp"
#include "autonomous.hpp"
#include "bot_connections.hpp"
#include "properties.hpp"
#include "vector2.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstdint>


namespace Controls {

	namespace {
		bool odom_initialized = false;
		std::int32_t last_lr_position = 0;
		std::int32_t last_fb_position = 0;
		std::int32_t accumulated_lr_delta = 0;
		std::int32_t accumulated_fb_delta = 0;
		int odom_sample_elapsed_msec = 0;
		float odom_turn_correction = 0.0f;

		int clamp_motor_input(int input) {
			return std::max(-127, std::min(input, 127));
		}
	}

	// Each of the functions below are getting called every 10 milliseconds within a while loop.

	void processDrive() {
		pros::Controller& controller = BotConnections::controller;

		int forward_input = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
		int turn_input = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);

		const int deadband = 10;
		if (forward_input > -deadband && forward_input < deadband) forward_input = 0;
		if (turn_input > -deadband && turn_input < deadband) turn_input = 0;

		const std::int32_t lr_position = BotConnections::LRODOM.get_position();
		const std::int32_t fb_position = BotConnections::FBODOM.get_position();

		if (!odom_initialized) {
			last_lr_position = lr_position;
			last_fb_position = fb_position;
			odom_initialized = true;
		}

		if (forward_input != 0 && turn_input == 0) {
			accumulated_lr_delta += lr_position - last_lr_position;
			accumulated_fb_delta += fb_position - last_fb_position;
			odom_sample_elapsed_msec += Properties::TICK_DELAY_MSEC;

			if (odom_sample_elapsed_msec >= Properties::ODOM_SAMPLE_DELAY_MSEC) {
				if (std::abs(accumulated_fb_delta) >= Properties::ODOM_MIN_FB_DELTA_CDEG) {
					const float lateral_to_forward_ratio =
						static_cast<float>(accumulated_lr_delta)
						/ static_cast<float>(std::abs(accumulated_fb_delta));
					const float target_correction = std::max(
						-static_cast<float>(Properties::ODOM_MAX_TURN_CORRECTION),
						std::min(
							-forward_input * Properties::LR_ODOM_DIRECTION
								* lateral_to_forward_ratio
								* Properties::ODOM_DRIVE_CORRECTION_GAIN,
							static_cast<float>(Properties::ODOM_MAX_TURN_CORRECTION)
						)
					);
					odom_turn_correction = 0.5f * odom_turn_correction
						+ 0.5f * target_correction;
				}

				accumulated_lr_delta = 0;
				accumulated_fb_delta = 0;
				odom_sample_elapsed_msec = 0;
			}
		} else {
			accumulated_lr_delta = 0;
			accumulated_fb_delta = 0;
			odom_sample_elapsed_msec = 0;
			odom_turn_correction = 0.0f;
		}

		last_lr_position = lr_position;
		last_fb_position = fb_position;

		const int corrected_turn_input = turn_input
			+ static_cast<int>(odom_turn_correction);
		const int left_input = forward_input + corrected_turn_input;
		const int right_input = forward_input - corrected_turn_input;

		BotConnections::left_mg.move(clamp_motor_input(
			static_cast<int>(left_input * Properties::LEFT_DRIVE_BIAS)
		));
		BotConnections::right_mg.move(clamp_motor_input(
			static_cast<int>(right_input * Properties::RIGHT_DRIVE_BIAS)
		));
	}

	void processLondon() {
		pros::Controller& controller = BotConnections::controller;
		if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_UP)) {
			BotConnections::LondonLift.move(Properties::MAX_MOTOR_VOLTS);
		} else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN)) {
			BotConnections::LondonLift.move(-Properties::MAX_MOTOR_VOLTS);
		} else {
			BotConnections::LondonLift.brake();
		}
	}

}