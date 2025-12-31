#include "main.h"


/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
	BotConnections::initialize();
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
	// Code snippet of routine being created and inserted in list of routines.
	std::shared_ptr<Autonomous::Routine> routine;
	routine->first = "First Routine!";
	routine->second.push(std::make_shared<Autonomous::Align>(Autonomous::Align::Config{}));
	Autonomous::routines.push_back(routine);

	// AUTONOMOUS START.
	Autonomous::initialize_actions_queue();

	while (true) {
		pros::delay(Properties::TICK_DELAY_MSEC);

		if (Autonomous::run_action_ticks() == Autonomous::ACTIONS_UNBLOCKED) {
			Autonomous::start_next_action();
		}
	}
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	// CONTROLS START.
	while (true) {
		pros::delay(Properties::TICK_DELAY_MSEC);

		Controls::processLeftJoystick();
		Controls::processRightJoystick();
		Controls::processLeftTriggers();
		Controls::processRightTriggers();
		Controls::processLeftButtons();
		Controls::processRightButtons();
	}
}