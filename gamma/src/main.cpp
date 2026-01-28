#include "main.h"


/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	BotConnections::initialize();
	Autonomous::load_routine_files();
	Autonomous::load_cache_file();
	OnboardGUI::initialize_selector();
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
void competition_initialize() {
	
}

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
	using namespace Autonomous;

	// AUTONOMOUS START.
	initialize_actions_queue();

	while (true) {
		if (run_action_ticks() == ACTIONS_UNBLOCKED) {
			start_next_action();
		}

<<<<<<< HEAD
		run_odom_tick();
=======
		// int curr_refresh_cycles = Properties::global_time_msec / Properties::SCREEN_REFRESH_DELAY_MSEC;
		// if (curr_refresh_cycles > Properties::screen_refresh_cycles) {
		// 	OnboardGUI::running_actions_view_run_tick();
		// 	Properties::screen_refresh_cycles = curr_refresh_cycles;
		// }
>>>>>>> parent of 80a4cc4 (Added odometry and Pursue action.)

		pros::delay(Properties::TICK_DELAY_MSEC);
		Properties::global_time_msec += Properties::TICK_DELAY_MSEC;
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
	using namespace Controls;

	// CONTROLS START.
	while (true) {
		pros::delay(Properties::TICK_DELAY_MSEC);

		processDrive();
		processIntake();
		processHood();
		processToggles();
		processPark();
	}
}
