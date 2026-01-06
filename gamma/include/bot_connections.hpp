#pragma once

#include "api.h"


namespace BotConnections {
	
	extern pros::Controller controller;

	extern pros::MotorGroup left_mg;
	extern pros::MotorGroup right_mg;

	extern pros::Imu imu;
	extern pros::Rotation odom;

	extern pros::Motor intake_A;
	extern pros::Motor intake_B;
	extern pros::Motor intake_C;

	extern pros::adi::DigitalOut park_mech;
	extern pros::adi::DigitalOut hood;
	extern pros::adi::DigitalOut flap;
	extern pros::adi::DigitalOut descore;
	extern pros::adi::DigitalOut unloader;

	extern pros::Distance dist_sens;
	extern pros::Optical vis_sens;

	
	void initialize();
}