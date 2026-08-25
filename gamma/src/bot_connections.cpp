#include "bot_connections.hpp"


namespace BotConnections {
	
	pros::Controller controller(pros::E_CONTROLLER_MASTER);

	pros::MotorGroup left_mg({1});
	pros::MotorGroup right_mg({4});

	pros::Imu imu(7);
	pros::Rotation LRODOM(8);
	pros::Rotation FBODOM(9);

	pros::Motor intake(-10);\
	pros::MotorGroup LondonLift({11,-12});


	pros::adi::DigitalOut AaronArm('A');



	void initialize() {
		imu.reset(true);

		left_mg.set_brake_mode_all(pros::E_MOTOR_BRAKE_HOLD);
		right_mg.set_brake_mode_all(pros::E_MOTOR_BRAKE_HOLD);

		left_mg.set_gearing_all(pros::E_MOTOR_GEAR_BLUE);
		right_mg.set_gearing_all(pros::E_MOTOR_GEAR_BLUE);
		LondonLift.set_gearing_all(pros::E_MOTOR_GEAR_RED);//84:12

		intake.set_encoder_units(pros::E_MOTOR_ENCODER_ROTATIONS);

		intake.set_gearing(pros::E_MOTOR_GEAR_GREEN);


		intake.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);


		vis_sens.set_led_pwm(100);
	}
}
