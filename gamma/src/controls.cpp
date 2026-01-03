#include "controls.hpp"
#include "bot_connections.hpp"
#include "vector2.hpp"


namespace Controls {
	static bool descore_state = false;
	static bool park_state    = false;
	static bool unloader_state = false;


	enum class IntakeMode {
		REVERSE,
		HIGH,
		LOW,
		OFF
	};

	void intakeWithColorSort(IntakeMode mode) {
		double hue = BotConnections::vis_sens.get_hue();

		bool is_red  = (hue >= 350 || hue <= 10);
		bool is_blue = (hue >= 200 && hue <= 220);

		// Backdoor DEFINITE WIP
		if (is_red || is_blue) {
			BotConnections::flap.set_value(true);
		} else {
			BotConnections::flap.set_value(false);
		}

		switch (mode) {
			case IntakeMode::REVERSE:
				BotConnections::intake_A.move(-127);
				BotConnections::intake_B.move(-127);
				break;

			case IntakeMode::HIGH:
				BotConnections::intake_A.move(127);
				BotConnections::intake_B.move(127);
				BotConnections::intake_C.move(127);
				break;
	
			case IntakeMode::LOW:
				BotConnections::intake_A.move(127);
				BotConnections::intake_B.move(127);
				BotConnections::intake_C.move(-127);
				break;
	
			default:
				BotConnections::intake_A.brake();
				BotConnections::intake_B.brake();
				BotConnections::intake_C.brake();
				BotConnections::flap.set_value(false);
				break;
		}
	}

	// Each of the functions below are getting called every 10 milliseconds within a while loop.

	void processLeftJoystick() {
		pros::Controller& controller = BotConnections::controller;

		Vector2 input {
			controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_X),
			controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y)
		};

		// Do something with the input.
	}


	void processRightJoystick() {
		pros::Controller& controller = BotConnections::controller;

		Vector2 input {
			controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X),
			controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y)
		};

		// Do something with the input.
	}


	void processLeftTriggers() {
		pros::Controller& controller = BotConnections::controller;

		// Currently Pressed.
		bool L1_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1);
		bool L2_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2);
		// Just Pressed.
		bool L1_pressed = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L1);
		bool L2_pressed = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L2);
		// Just Released.
		bool L1_released = controller.get_digital_new_release(pros::E_CONTROLLER_DIGITAL_L1);
		bool L2_released = controller.get_digital_new_release(pros::E_CONTROLLER_DIGITAL_L2);

		// If statements below utilizing the conditions above.
	}


	void processRightTriggers() {
		pros::Controller& controller = BotConnections::controller;

		// Currently Pressed.
		bool R1_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1);
		bool R2_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2);
		// Just Pressed.
		bool R1_pressed = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_R1);
		bool R2_pressed = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_R2);
		// Just Released.
		bool R1_released = controller.get_digital_new_release(pros::E_CONTROLLER_DIGITAL_R1);
		bool R2_released = controller.get_digital_new_release(pros::E_CONTROLLER_DIGITAL_R2);

		// If statements below utilizing the conditions above.
	}


	void processLeftButtons() {
		pros::Controller& controller = BotConnections::controller;

		// Currently Pressed.
		bool up_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_UP);
		bool down_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN);
		bool left_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT);
		bool right_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT);
		// Just Pressed.
		bool up_pressed = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_UP);
		bool down_pressed = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN);
		bool left_pressed = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_LEFT);
		bool right_pressed = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_RIGHT);
		// Just Released.
		bool up_released = controller.get_digital_new_release(pros::E_CONTROLLER_DIGITAL_UP);
		bool down_released = controller.get_digital_new_release(pros::E_CONTROLLER_DIGITAL_DOWN);
		bool left_released = controller.get_digital_new_release(pros::E_CONTROLLER_DIGITAL_LEFT);
		bool right_released = controller.get_digital_new_release(pros::E_CONTROLLER_DIGITAL_RIGHT);

		// If statements below utilizing the conditions above.
	}


	void processRightButtons() {
		pros::Controller& controller = BotConnections::controller;

		// Currently Pressed.
		bool A_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_A);
		bool B_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_B);
		bool X_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_X);
		bool Y_on = controller.get_digital(pros::E_CONTROLLER_DIGITAL_Y);
		// Just Pressed.
		bool A_pressed = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A);
		bool B_pressed = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B);
		bool X_pressed = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_X);
		bool Y_pressed = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_Y);
		// Just Released.
		bool A_released = controller.get_digital_new_release(pros::E_CONTROLLER_DIGITAL_A);
		bool B_released = controller.get_digital_new_release(pros::E_CONTROLLER_DIGITAL_B);
		bool X_released = controller.get_digital_new_release(pros::E_CONTROLLER_DIGITAL_X);
		bool Y_released = controller.get_digital_new_release(pros::E_CONTROLLER_DIGITAL_Y);

		// If statements below utilizing the conditions above.
	}
}
