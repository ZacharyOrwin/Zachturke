#include "autonomous.hpp"
#include "bot_connections.hpp"
#include "properties.hpp"
#include "xmath.hpp"
#include "vector2.hpp"


namespace Autonomous {

	ActionRunStatus Toggleable::process_toggle(bool& toggle_state, float time, std::function<void()> exit_func) {
		if (
			toggling && !toggle_state
			|| !toggling && (time > timeout || !toggle_state)
		) {
			exit_func();
			toggle_state = false;
			return ACTION_RUN_COMPLETE;

		} else {
			return ACTION_RUN_ONGOING;
		}
	}


	ActionRunStatus Align::run_tick() {
		// Input.
                Config* cfg = static_cast<Config*>(c.get());
		
		pros::Imu& imu = BotConnections::imu;
		pros::MotorGroup& left_mg = BotConnections::left_mg;
		pros::MotorGroup& right_mg = BotConnections::right_mg;

		float curr_ang = imu.get_heading();
        	float curr_rad = XMath::rad(curr_ang);
        	float target_rad = XMath::rad(cfg->angle);

		Vector2 curr_face_vec = Vector2 {
                	std::cos(curr_rad),
			std::sin(curr_rad)
		};
		Vector2 curr_right_vec = Vector2 {
			std::sin(curr_rad),
			-std::cos(curr_rad)
		};
		Vector2 target_face_vec = Vector2 {
			std::cos(target_rad),
			std::sin(target_rad),
		};

		float curr_face_dot_target_face = curr_face_vec.dot(target_face_vec);
		float curr_right_dot_target_face = curr_right_vec.dot(target_face_vec);
		// Range [0, 1] : 0 is target_face and curr_face aligned, 1 is them facing
		// opposite directions.
		float offset = (1 - curr_face_dot_target_face) / 2.0;

		// Exit Condition.
		if (time > cfg->timeout || offset < cfg->epsilon) {
			left_mg.brake();
			right_mg.brake();
			return ACTION_RUN_COMPLETE;
		}

		// Output.
		int sign = XMath::sgn<float>(curr_right_dot_target_face);
		float twist = sign * cfg->kba.output(offset);

		left_mg.move(-twist);
		right_mg.move(twist);

		time += Properties::TICK_DELAY_MSEC/1000.0;

		return ACTION_RUN_ONGOING;
	}


	ActionRunStatus Travel::run_tick() {
		// Input.
                Config* cfg = static_cast<Config*>(c.get());

		pros::MotorGroup& left_mg = BotConnections::left_mg;
		pros::MotorGroup& right_mg = BotConnections::right_mg;

		float left_vel = left_mg.get_actual_velocity();
		float right_vel = right_mg.get_actual_velocity();
		float avg_vel = Properties::FINAL_DRIVE_RATIO
			* XMath::rpmToRads((left_vel + right_vel) / 2);

		float diff = cfg->dist - accum_dist;

		// Exit Condition.
		if (time > cfg->timeout || std::abs(diff) < cfg->epsilon) {
			left_mg.brake();
			right_mg.brake();
			return ACTION_RUN_COMPLETE;
		}

		// Output.
		float push = XMath::sgn<float>(diff) * cfg->kba.output(std::abs(diff));

		left_mg.move(push);
		right_mg.move(push);

		accum_dist += Properties::WHEEL_DIAMETER_IN/2
			* avg_vel
			* Properties::TICK_DELAY_MSEC/1000.0;
		
		time += Properties::TICK_DELAY_MSEC/1000.0;

		return ACTION_RUN_ONGOING;
	}

	
	ActionRunStatus ColorSort::run_tick() {
		// Input.
		Config* cfg = static_cast<Config*>(c.get());

		pros::adi::DigitalOut& flap = BotConnections::flap;

		double hue = BotConnections::vis_sens.get_hue();

		// Exit Condition.
		std::function<void()> exit_f = [&flap]() -> void {
			flap.set_value(false);
		};
		if (cfg->process_toggle(toggle_state, time, exit_f) == ACTION_RUN_COMPLETE) {
			return ACTION_RUN_COMPLETE;
		}

		// Output.
		bool is_red = (hue >= 350 || hue <= 10);
		bool is_blue = (hue >= 200 && hue <= 220);

		// If the color to sort out is detected at the sensor.
		if (is_blue && cfg->color == Properties::COLOR_SORT_BLUE
			|| is_red && cfg->color == Properties::COLOR_SORT_RED
		) {
			flap_open = true;
			flap.set_value(flap_open);
			closing = false;

		} else if (closing) {
			float delta = BotConnections::intake_A.get_position() - last_open_ang;

			if (delta > cfg->closing_rotations) {
				flap_open = false;
				flap.set_value(flap_open);
				closing = false;
			}

		} else if (flap_open){
			closing = true;
			last_open_ang = BotConnections::intake_A.get_position();
		}

		time += Properties::TICK_DELAY_MSEC/1000.0;
		
		return ACTION_RUN_ONGOING;
	}


	ActionRunStatus Intake::run_tick() {
		// Input.
		Config* cfg = static_cast<Config*>(c.get());

		pros::Motor& intake_A = BotConnections::intake_A;
		pros::Motor& intake_B = BotConnections::intake_B;
		pros::Motor& intake_C = BotConnections::intake_C;

		Properties::intake_mode = cfg->intake_mode;

		// Exit Condition.
		std::function<void()> exit_f = [&intake_A, &intake_B, &intake_C]() -> void {
			intake_A.brake();
			intake_B.brake();
			intake_C.brake();
		};
		if (cfg->process_toggle(toggle_state, time, exit_f) == ACTION_RUN_COMPLETE) {
			return ACTION_RUN_COMPLETE;
		}

		// Output.
		switch (Properties::intake_mode) {
			case Properties::INTAKE_REVERSE:
				intake_A.move(-Properties::MAX_MOTOR_VOLTS);
				intake_B.move(-Properties::MAX_MOTOR_VOLTS);
				break;

			case Properties::INTAKE_TOP:
				intake_A.move(Properties::MAX_MOTOR_VOLTS);
				intake_B.move(Properties::MAX_MOTOR_VOLTS);
				intake_C.move(Properties::MAX_MOTOR_VOLTS);
				break;
	
			case Properties::INTAKE_BOTTOM:
				intake_A.move(Properties::MAX_MOTOR_VOLTS);
				intake_B.move(Properties::MAX_MOTOR_VOLTS);
				intake_C.move(-Properties::MAX_MOTOR_VOLTS);
				break;
	
			case Properties::INTAKE_OFF:
				intake_A.brake();
				intake_B.brake();
				intake_C.brake();
				break;
		}

		time += Properties::TICK_DELAY_MSEC/1000.0;

		return ACTION_RUN_ONGOING;
	}


	ActionRunStatus Hood::run_tick() {
		// Input.
		Config* cfg = static_cast<Config*>(c.get());

		pros::adi::DigitalOut& hood = BotConnections::hood;

		// Exit Condition.
		std::function<void()> exit_f = [&hood]() -> void {
			hood.set_value(false);
		};
		if (cfg->process_toggle(toggle_state, time, exit_f) == ACTION_RUN_COMPLETE) {
			return ACTION_RUN_COMPLETE;
		}

		hood.set_value(true);

		time += Properties::TICK_DELAY_MSEC/1000.0;

		return ACTION_RUN_ONGOING;
	}


	ActionRunStatus Unloader::run_tick() {
		// Input.
		Config* cfg = static_cast<Config*>(c.get());

		pros::adi::DigitalOut& unloader = BotConnections::unloader;

		// Exit Condition.
		std::function<void()> exit_f = [&unloader]() -> void {
			unloader.set_value(false);
		};
		if (cfg->process_toggle(toggle_state, time, exit_f) == ACTION_RUN_COMPLETE) {
			return ACTION_RUN_COMPLETE;
		}

		unloader.set_value(true);

		time += Properties::TICK_DELAY_MSEC/1000.0;

		return ACTION_RUN_ONGOING;
	}


	ActionRunStatus Descore::run_tick() {
		// Input.
		Config* cfg = static_cast<Config*>(c.get());

		pros::adi::DigitalOut& descore = BotConnections::descore;

		// Exit Condition.
		std::function<void()> exit_f = [&descore]() -> void {
			descore.set_value(false);
		};
		if (cfg->process_toggle(toggle_state, time, exit_f) == ACTION_RUN_COMPLETE) {
			return ACTION_RUN_COMPLETE;
		}

		descore.set_value(true);

		time += Properties::TICK_DELAY_MSEC/1000.0;

		return ACTION_RUN_ONGOING;
	}


	void Align::start() {}


	void Travel::start() {}


	void ColorSort::start() {
		Config* cfg = static_cast<Config*>(c.get());

		if (cfg->toggling) {
			toggle_state = !toggle_state;
		} else {
			toggle_state = true;
		}
	}


	void Intake::start() {
		Config* cfg = static_cast<Config*>(c.get());

		if (cfg->toggling) {
			toggle_state = !toggle_state;
		} else {
			toggle_state = true;
		}
	}


	void Hood::start() {}


	void Unloader::start() {}


	void Descore::start() {}


	std::string get_action_type(std::shared_ptr<Action>& action) {
		if (std::dynamic_pointer_cast<Align>(action)) {
			return "ALIGN";

		} else if (std::dynamic_pointer_cast<Travel>(action)) {
			return "TRAVEL";
			
		} else if (std::dynamic_pointer_cast<ColorSort>(action)) {
			return "COLSORT";
			
		} else if (std::dynamic_pointer_cast<Intake>(action)) {
			return "INTAKE";
			
		} else {
			return "UNKNOWN";
		}
	}
}