#pragma once

#include "api.h"


namespace BotConnections {
	
	extern pros::Controller controller;
	extern pros::MotorGroup left_mg;
	extern pros::MotorGroup right_mg;
	extern pros::Imu imu;
	extern pros::Rotation LRODOM;
	extern pros::Rotation FBODOM;
	extern pros::Motor intake;
	extern pros::MotorGroup LondonLift;
	
	void initialize();
}