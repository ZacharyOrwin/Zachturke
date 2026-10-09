#pragma once

#include "api.h"


namespace Properties {
	

	inline constexpr int TICK_DELAY_MSEC = 5;
	inline constexpr int SCREEN_REFRESH_DELAY_MSEC = 400;
	inline constexpr int MAX_MOTOR_VOLTS = 127;
	inline constexpr float FINAL_DRIVE_RATIO = 3.0/4.0 /*2/3.0*/;
	inline constexpr float WHEEL_DIAMETER_IN = 3.25;
	inline constexpr float LEFT_DRIVE_BIAS = 1.0 ;
	inline constexpr float RIGHT_DRIVE_BIAS = 1.0;
	inline constexpr float LR_ODOM_DIRECTION = 1.0;
	inline constexpr float FB_ODOM_DIRECTION = 1.0;
	inline constexpr float RECORDED_AUTON_LOOKAHEAD_CDEG = 2500.0;
	inline constexpr float ODOM_DRIVE_CORRECTION_GAIN = 0.5;
	inline constexpr int ODOM_MAX_TURN_CORRECTION = 24;
	inline constexpr int ODOM_SAMPLE_DELAY_MSEC = 100;
	inline constexpr int ODOM_MIN_FB_DELTA_CDEG = 5;
	inline int global_time_msec = 0;
	inline int screen_refresh_cycles = 0;

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