#include "autonomous.hpp"
#include "properties.hpp"


namespace Autonomous {
	std::shared_ptr<Routine> active_routine;
	std::vector<std::shared_ptr<Routine>> routines;
	std::queue<std::shared_ptr<Action>> actions_queue;
	std::vector<std::shared_ptr<Action>> actions_running;


	Action::Action(Align::Config c) {this->c = c;}


	ActionRunStatus Align::run_tick() {
		time += Properties::TICK_DELAY_MSEC/1000.0;

		if (time > c.timeout) return ACTION_RUN_COMPLETE;

		return ACTION_RUN_ONGOING;
	}


	ActionRunStatus Travel::run_tick() {
		time += Properties::TICK_DELAY_MSEC/1000.0;

		if (time > c.timeout) return ACTION_RUN_COMPLETE;

		return ACTION_RUN_ONGOING;
	}


	void select_routine(int index) {
		active_routine = routines.at(index);
	}


	void initialize_actions_queue() {
		actions_queue = active_routine->second;
	}


	void start_next_action() {
		actions_running.push_back(actions_queue.front());
		actions_queue.pop();
	}


	ActionsBlockingStatus run_action_ticks() {
		int current_index = 0;
		bool blocking = false;

		for (const auto& action : actions_running) {
			if (!blocking && action->blocking) blocking = true;

			if (action->run_tick() == ACTION_RUN_COMPLETE) {
				actions_running.erase(actions_running.begin() + current_index);
			} else {
				current_index++;
			}
		}

		return blocking ? ACTIONS_BLOCKED : ACTIONS_UNBLOCKED;
	}
}