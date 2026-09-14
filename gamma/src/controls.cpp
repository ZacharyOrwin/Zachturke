#include "controls.hpp"
#include "autonomous.hpp"
#include "bot_connections.hpp"
#include "properties.hpp"
#include "vector2.hpp"


namespace Controls {

	// Each of the functions below are getting called every 10 milliseconds within a while loop.

	void processDrive() {
		pros::Controller& controller = BotConnections::controller;

		int left_input = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
		int right_input = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);

		const int deadband = 10;
		if (left_input > -deadband && left_input < deadband) left_input = 0;
		if (right_input > -deadband && right_input < deadband) right_input = 0;

		BotConnections::left_mg.move(left_input * Properties::LEFT_DRIVE_BIAS);
		BotConnections::right_mg.move(right_input * Properties::RIGHT_DRIVE_BIAS);
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