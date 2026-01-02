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

		BConfig(bool b = true) : blocking(b) {}
		virtual ~BConfig() = default;
	};

	struct Action {
		float time = 0;

		std::unique_ptr<BConfig> c;
                Action(std::unique_ptr<BConfig> c) : c(std::move(c)) {}
		virtual ~Action() = default;
		virtual ActionRunStatus run_tick() = 0;
	};

	struct Align : public Action {
		struct Config : public BConfig {
			KBA kba;
			float angle;
			float epsilon;
			float timeout;

			Config(KBA k, float a, float e, float t, bool b = true) :
				BConfig(b), kba(k), angle(a), epsilon(e), timeout(t) {}
		};

		Align(Config c) : Action(std::make_unique<Config>(std::move(c))) {}
		ActionRunStatus run_tick() override;
	};

	struct Travel : public Action {
		struct Config : public BConfig {
			KBA kba;
			float dist;
			float epsilon;
			float timeout;

			Config(KBA k, float d, float e, float t, bool b = true) :
				BConfig(b), kba(k), dist(d), epsilon(e), timeout(t) {}
		};

		Travel(Config c) : Action(std::make_unique<Config>(std::move(c))) {}
		ActionRunStatus run_tick() override;
	};

	typedef std::pair<std::string, std::queue<std::shared_ptr<Action>>> Routine;

	inline Routine* active_routine = nullptr;
	inline std::vector<std::shared_ptr<Routine>> routines; // <Name (of routine), Routine>
	inline std::queue<std::shared_ptr<Action>> actions_queue;
	inline std::vector<std::shared_ptr<Action>> actions_running;

	void select_routine(int index);
	void initialize_actions_queue();
	void start_next_action();
	ActionsBlockingStatus run_action_ticks();
}