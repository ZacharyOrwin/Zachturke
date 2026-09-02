#include "controls.hpp"
#include "autonomous.hpp"
#include "bot_connections.cpp"
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


	


	void processLondon() {
		pros::Controller& controller = BotConnections::controller;
		if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_UP)){
			BotConnections::LondonLift.move(Properties::MAX_MOTOR_VOLTS);
		}
	}


}