
#include "autonomous.hpp"


namespace Autonomous {

	void select_routine(int index) {
		active_routine = routines.at(index).get();
	}


	void select_routine(std::string name) {
		for (auto& routine : routines) {
			if (routine->first == name) {
				active_routine = routine.get();
				return;
			}
		}
	}


	void initialize_actions_queue() {
		actions_queue = active_routine->second;
	}


	void start_next_action() {
		if (!actions_queue.empty()) {
                        auto& action = actions_queue.front();

                        action->start();

			actions_running.push_back(action);
			actions_queue.pop();
		}
	}


	ActionsBlockingStatus run_action_ticks() {
		bool blocking = false;

                printf("A\n");
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