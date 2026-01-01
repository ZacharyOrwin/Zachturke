#pragma once

#include "api.h"
#include <queue>


namespace Autonomous {
	enum ActionRunStatus {
		ACTION_RUN_ONGOING,
		ACTION_RUN_COMPLETE
	};
	enum ActionsBlockingStatus {
		ACTIONS_BLOCKED,
		ACTIONS_UNBLOCKED
	};

	struct KBA {
		float k;
		float b;
		float a;

		float output(float t) {
			return std::min(std::max(t*k, a), b);
		}
	};

	struct BConfig {
		bool blocking = true;
	};

	struct Action {
		float time = 0;

		BConfig c;

		Action(BConfig c);
		virtual ~Action() = default;
		virtual ActionRunStatus run_tick() = 0;
	};

	struct Align : public Action {
		struct Config {
			KBA kba;
			float angle;
			float epsilon;
			float timeout;
		};

		Config c;

		Align(Config c);
		ActionRunStatus run_tick() override;
	};

	struct Travel : public Action {
		struct Config {
			KBA kba;
			float dist;
			float epsilon;
			float timeout;
		};

		Config c;

		Travel(Config c);
		ActionRunStatus run_tick() override;
	};

	typedef std::pair<std::string, std::queue<std::shared_ptr<Action>>> Routine;

	extern std::shared_ptr<Routine> active_routine;
	extern std::vector<std::shared_ptr<Routine>> routines; // <Name (of routine), Routine>
	extern std::queue<std::shared_ptr<Action>> actions_queue;
	extern std::vector<std::shared_ptr<Action>> actions_running;

	void select_routine(int index);
	void initialize_actions_queue();
	void start_next_action();
	ActionsBlockingStatus run_action_ticks();
}