#include "bot_connections.hpp"


namespace BotConnections {
	
	pros::Controller controller(pros::E_CONTROLLER_MASTER);

	pros::MotorGroup left_mg({1} /*{-7,3,-2}*/);
	pros::MotorGroup right_mg({-10} /*{8,-9,10}*/);

	pros::Imu imu(2 /*4*/);
	pros::Rotation odom(6);

	pros::Motor intake_A(-13);
	pros::Motor intake_B(11);
	pros::Motor intake_C(12);

	pros::adi::DigitalOut park_mech('A');
	pros::adi::DigitalOut hood('B');
	pros::adi::DigitalOut flap('C');
	pros::adi::DigitalOut descore('D');
	pros::adi::DigitalOut unloader('E');

	pros::Distance dist_sens(20);
	pros::Optical vis_sens(1);


	void initialize() {
		imu.reset(true);

		left_mg.set_gearing_all(pros::E_MOTOR_GEAR_BLUE);
		right_mg.set_gearing_all(pros::E_MOTOR_GEAR_BLUE);

		intake_A.set_gearing(pros::E_MOTOR_GEAR_GREEN);
		intake_B.set_gearing(pros::E_MOTOR_GEAR_BLUE);
		intake_C.set_gearing(pros::E_MOTOR_GEAR_BLUE);

		vis_sens.set_led_pwm(100);
	}
}
