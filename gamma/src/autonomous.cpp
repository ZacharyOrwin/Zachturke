#include "autonomous.hpp"
#include "properties.hpp"


namespace Autonomous {

	ActionRunStatus Align::run_tick() {
                Config* cfg = static_cast<Config*>(c.get());

		time += Properties::TICK_DELAY_MSEC/1000.0;

		if (time > cfg->timeout) return ACTION_RUN_COMPLETE;

		return ACTION_RUN_ONGOING;
	}


	ActionRunStatus Travel::run_tick() {
                Config* cfg = static_cast<Config*>(c.get());

		time += Properties::TICK_DELAY_MSEC/1000.0;

		if (time > cfg->timeout) return ACTION_RUN_COMPLETE;

		return ACTION_RUN_ONGOING;
	}


	void select_routine(int index) {
		active_routine = routines.at(index).get();
	}


	void initialize_actions_queue() {
		actions_queue = active_routine->second;
	}


	void start_next_action() {
		if (!actions_queue.empty()) {
			actions_running.push_back(actions_queue.front());
			actions_queue.pop();
		}
	}


	ActionsBlockingStatus run_action_ticks() {
		bool blocking = false;

		for (auto it = actions_running.begin(); it != actions_running.end(); ) {
			auto& action = *it;

			if (action->run_tick() == ACTION_RUN_COMPLETE) {
				it = actions_running.erase(it);
			} else {
				if (!blocking && action->c->blocking) blocking = true;
				++it;
			}
		}

		return blocking ? ACTIONS_BLOCKED : ACTIONS_UNBLOCKED;
	}
}