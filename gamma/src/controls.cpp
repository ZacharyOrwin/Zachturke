#include "controls.hpp"
#include "autonomous.hpp"
#include "bot_connections.hpp"
#include "properties.hpp"
#include "vector2.hpp"


namespace Controls {

	// Each of the functions below are getting called every 10 milliseconds within a while loop.

	void processDrive() {
		pros::Controller& controller = BotConnections::controller;

		int Y = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
		int X = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);

		int left_power  = Y + X;
		int right_power = Y - X;

		BotConnections::left_mg.move(left_power * Properties::LEFT_DRIVE_BIAS);
		BotConnections::right_mg.move(right_power * Properties::RIGHT_DRIVE_BIAS);
	}


	void processIntake() {
		pros::Controller& controller = BotConnections::controller;
		pros::Motor& intake_A = BotConnections::intake_A;
		pros::Motor& intake_B = BotConnections::intake_B;
		pros::Motor& intake_C = BotConnections::intake_C;

		bool R1_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1);
		bool L1_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1);
		bool L2_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2);

		// Priority: R1 > L1 > L2 > Off
		if (R1_on) {
			Properties::intake_mode = Properties::INTAKE_REVERSE;
			Autonomous::Intake::toggle_state = true;
		} else if (L1_on) {
			Properties::intake_mode = Properties::INTAKE_TOP;
			Autonomous::Intake::toggle_state = true;
		} else if (L2_on) {
			Properties::intake_mode = Properties::INTAKE_BOTTOM;
			Autonomous::Intake::toggle_state = true;
		} else {
			Properties::intake_mode = Properties::INTAKE_OFF;
			Autonomous::Intake::toggle_state = false;
		}

		switch (Properties::intake_mode) {
			case Properties::INTAKE_REVERSE:
				intake_A.move(-Properties::MAX_MOTOR_VOLTS);
				intake_B.move(-Properties::MAX_MOTOR_VOLTS);
				break;

			case Properties::INTAKE_TOP:
				intake_A.move(Properties::MAX_MOTOR_VOLTS);
				intake_B.move(Properties::MAX_MOTOR_VOLTS);

				if (intake_C.get_efficiency() > 5) {
					intake_C.move(Properties::MAX_MOTOR_VOLTS);
				} else {
					intake_C.brake();
				}

				break;
	
			case Properties::INTAKE_BOTTOM:
				intake_A.move(Properties::MAX_MOTOR_VOLTS);
				intake_B.move(Properties::MAX_MOTOR_VOLTS);
				intake_C.move(-Properties::MAX_MOTOR_VOLTS);
				break;
	
			case Properties::INTAKE_OFF:
				intake_A.brake();
				intake_B.brake();
				intake_C.brake();
				break;
		}
	}


	void processLondon() {
		pros::Controller& controller = BotConnections::controller;
		if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_UP)){
			BotConnections::LondonLift.move(Properties::MAX_MOTOR_VOLTS);
		}
	}

	void processToggles() {
		pros::Controller& controller = BotConnections::controller;

		static bool lil_will_state = false;
		static bool descore_state = false;

		if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B)) {
			lil_will_state = !lil_will_state;
			BotConnections::unloader.set_value(lil_will_state);
		}

		if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A)) {
			descore_state = !descore_state;
			BotConnections::descore.set_value(descore_state);
		}
	}


	void processPark() {
		pros::Controller& controller = BotConnections::controller;

		bool park_state = false;

		bool down_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN);
		bool up_on   = controller.get_digital(pros::E_CONTROLLER_DIGITAL_UP);

		double dist_mm = BotConnections::dist_sens.get_distance();

		if (down_on) {
			if (dist_mm >= 32) {
				BotConnections::intake_A.move(-77);
				BotConnections::intake_B.move(-77);
			} else if (!park_state) {
				BotConnections::intake_A.move_relative(-65, 40); // ~0.1 turns
				BotConnections::intake_B.move_relative(-65, 40);

				BotConnections::park_mech.set_value(true);
				park_state = true;
			}
		}

		if (up_on) {
			BotConnections::park_mech.set_value(false);
			park_state = false;
		}
	}
}