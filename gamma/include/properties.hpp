#pragma once

#include "api.h"
#include "vector2.hpp"


namespace Properties {
	
	enum IntakeMode {
		INTAKE_OFF,
		INTAKE_REVERSE,
		INTAKE_TOP,
		INTAKE_BOTTOM
	};

	enum ColorSortColor {
		COLOR_SORT_RED,
		COLOR_SORT_BLUE
	};

	inline constexpr int TICK_DELAY_MSEC = 10;
	inline constexpr int SCREEN_REFRESH_DELAY_MSEC = 400;
	inline constexpr int MAX_MOTOR_VOLTS = 127;
	inline constexpr float FINAL_DRIVE_RATIO = 3.0/4.0 /*2/3.0*/;
	inline constexpr float WHEEL_DIAMETER_IN = 3.25;
	inline constexpr float ODOM_DIAMETER_IN = 2.0;
	inline int global_time_msec = 0;
	inline int screen_refresh_cycles = 0;
	inline int odom_cdeg = 0;
	inline Vector2 odom_position {};
	inline IntakeMode intake_mode = INTAKE_TOP;

	inline float get_gear_ratio(pros::v5::MotorGears gear_set) {
		switch (gear_set)
		{
		case pros::v5::MotorGears::ratio_6_to_1:
			return 6.0;
		case pros::v5::MotorGears::ratio_18_to_1:
                        return 18.0;
		case pros::v5::MotorGears::ratio_36_to_1:
                        return 36.0;
		default:
			return 1.0;
		}
	}
}