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
		float twist = sign * cfg->kbat.output(offset, time);

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
		float push = XMath::sgn<float>(diff) * cfg->kbat.output(std::abs(diff), time);

		left_mg.move(push * Properties::LEFT_DRIVE_BIAS);
		right_mg.move(push * Properties::RIGHT_DRIVE_BIAS);

		accum_dist += Properties::WHEEL_DIAMETER_IN/2
			* avg_vel
			* Properties::TICK_DELAY_MSEC/1000.0;
		
		time += Properties::TICK_DELAY_MSEC/1000.0;

		return ACTION_RUN_ONGOING;
	}

	


	void Align::start() {}


	void Travel::start() {}

	std::string get_action_type(std::shared_ptr<Action>& action) {
		if (std::dynamic_pointer_cast<Align>(action)) {
			return "ALIGN";

		} else if (std::dynamic_pointer_cast<Travel>(action)) {
			return "TRAVEL";
			
		} else {
			return "UNKNOWN";
		}
	}
}