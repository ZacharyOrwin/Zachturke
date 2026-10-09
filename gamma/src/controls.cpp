#include "controls.hpp"
#include "bot_connections.hpp"
#include "properties.hpp"
#include "recorded_auton.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>


namespace Controls {

	namespace {
		bool odom_initialized = false;
		std::int32_t last_lr_position = 0;
		std::int32_t last_fb_position = 0;
		std::int32_t accumulated_lr_delta = 0;
		std::int32_t accumulated_fb_delta = 0;
		int odom_sample_elapsed_msec = 0;
		float odom_turn_correction = 0.0f;
		std::uint16_t previous_london_buttons = 0;
		bool london_position_control_active = false;
		bool previous_pneumatics_y = false;
		constexpr std::int32_t LONDON_POSITION_VELOCITY_RPM = 100;
		constexpr std::int32_t INTAKE_MAX_VELOCITY_RPM = 200;
		constexpr double LONDON_GEAR_REDUCTION = 84.0 / 12.0;

		int clamp_motor_input(int input) {
			return std::max(-127, std::min(input, 127));
		}

		void command_london_position(double position_degrees) {
			// Targets are lift-output degrees; the motor turns 84/12 times per output turn.
			const std::int32_t result = BotConnections::LondonLift.move_absolute(
				position_degrees * LONDON_GEAR_REDUCTION,
				LONDON_POSITION_VELOCITY_RPM
			);
			if (result == PROS_ERR) {
				std::printf("London lift position command failed.\n");
				BotConnections::LondonLift.brake();
				london_position_control_active = false;
				return;
			}
			london_position_control_active = true;
		}

		void process_london_buttons(std::uint16_t buttons) {
			const bool moving_manually =
				(buttons & (RecordedAuton::BUTTON_L2 | RecordedAuton::BUTTON_UP))
				|| (buttons & (RecordedAuton::BUTTON_L1 | RecordedAuton::BUTTON_DOWN));
			if (buttons & (RecordedAuton::BUTTON_L2 | RecordedAuton::BUTTON_UP)) {
				BotConnections::LondonLift.move(Properties::MAX_MOTOR_VOLTS);
				london_position_control_active = false;
			} else if (buttons & (RecordedAuton::BUTTON_L1 | RecordedAuton::BUTTON_DOWN)) {
				BotConnections::LondonLift.move(-Properties::MAX_MOTOR_VOLTS);
				london_position_control_active = false;
			}

			if (!moving_manually) {
				const std::uint16_t new_presses =
					buttons & static_cast<std::uint16_t>(~previous_london_buttons);
				if (new_presses & RecordedAuton::BUTTON_X) {
					command_london_position(0.0);
				} else if (new_presses & RecordedAuton::BUTTON_A) {
					command_london_position(90.0);
				} else if (new_presses & RecordedAuton::BUTTON_B) {
					command_london_position(180.0);
				} else if (!london_position_control_active) {
					BotConnections::LondonLift.brake();
				}
			}
			previous_london_buttons = buttons;
		}

		void process_pneumatics_y(bool pressed) {
			if (pressed && !previous_pneumatics_y) {
				const std::int32_t result = BotConnections::london_pneumatic.toggle();
				if (result == PROS_ERR) {
					std::printf("Pneumatic toggle failed.\n");
				}
			}
			previous_pneumatics_y = pressed;
		}

		void process_intake_buttons(std::uint16_t buttons) {
			const bool forward = (buttons & RecordedAuton::BUTTON_R1) != 0;
			const bool reverse = (buttons & RecordedAuton::BUTTON_R2) != 0;
			if (forward == reverse) {
				BotConnections::intake.brake();
				return;
			}

			const std::int32_t velocity = forward
				? INTAKE_MAX_VELOCITY_RPM
				: -INTAKE_MAX_VELOCITY_RPM;
			if (BotConnections::intake.move_velocity(velocity) == PROS_ERR) {
				std::printf("Intake velocity command failed.\n");
				BotConnections::intake.brake();
			}
		}
	}

	// Each function below runs once per configured control tick.

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
		std::uint16_t buttons = 0;
		if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
			buttons |= RecordedAuton::BUTTON_L1;
		}
		if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
			buttons |= RecordedAuton::BUTTON_L2;
		}
		if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_A)) {
			buttons |= RecordedAuton::BUTTON_A;
		}
		if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_B)) {
			buttons |= RecordedAuton::BUTTON_B;
		}
		if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_X)) {
			buttons |= RecordedAuton::BUTTON_X;
		}
		process_london_buttons(buttons);
	}

	void processLondonButtons(std::uint16_t buttons) {
		process_london_buttons(buttons);
	}

	void processIntake() {
		pros::Controller& controller = BotConnections::controller;
		std::uint16_t buttons = 0;
		if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
			buttons |= RecordedAuton::BUTTON_R1;
		}
		if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
			buttons |= RecordedAuton::BUTTON_R2;
		}
		process_intake_buttons(buttons);
	}

	void processIntakeButtons(std::uint16_t buttons) {
		process_intake_buttons(buttons);
	}

	void processPneumatics() {
		process_pneumatics_y(
			BotConnections::controller.get_digital(pros::E_CONTROLLER_DIGITAL_Y)
		);
	}

	void processPneumaticsButtons(std::uint16_t buttons) {
		process_pneumatics_y((buttons & RecordedAuton::BUTTON_Y) != 0);
	}

}