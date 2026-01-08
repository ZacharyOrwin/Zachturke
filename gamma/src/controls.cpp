#include "controls.hpp"
#include "bot_connections.hpp"
#include "vector2.hpp"


namespace Controls {

	// Each of the functions below are getting called every 10 milliseconds within a while loop.

	void processLeftJoystick() {
		pros::Controller& controller = BotConnections::controller;


		int Y = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);

		// Store Y somewhere accessible
		Properties::drive_y = Y;
	}


	void processRightJoystick() {
		pros::Controller& controller = BotConnections::controller;

			int X = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);

			Properties::drive_x = X;
		};

		// Do something with the input.
	}
	void processDrive() {
		int Y = Properties::drive_y;
		int X = Properties::drive_x;

		int left_power  = Y + X;
		int right_power = Y - X;

		BotConnections::left_mg.move(left_power);
		BotConnections::right_mg.move(right_power);
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
