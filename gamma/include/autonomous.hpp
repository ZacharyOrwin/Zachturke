#pragma once

#include "api.h"
#include <queue>
#include <filesystem>


namespace Autonomous {

	enum ActionRunStatus {
		ACTION_RUN_ONGOING,
		ACTION_RUN_COMPLETE
	};
	enum ActionsBlockingStatus {
		ACTIONS_BLOCKED,
		ACTIONS_UNBLOCKED
	};

	typedef std::string ParameterToken, ValueToken;

	struct KBA {
		float k;
		float b;
		float a;

		float output(float t) {
			return std::min(std::max(t*k, a), b);
		}

		static KBA parse(ValueToken v);
	};

	struct BConfig {
		bool blocking = true;

		BConfig(bool b = true) : blocking(b) {}
		virtual ~BConfig() = default;
	};

	struct Action {
		float time = 0;

		std::shared_ptr<BConfig> c;
                Action(std::shared_ptr<BConfig> c) : c(std::move(c)) {}
		virtual ~Action() = default;
		virtual ActionRunStatus run_tick() = 0;
	};

	struct Align : public Action {
		struct Config : public BConfig {
			KBA kba;
			float angle;
			float epsilon;
			float timeout;

			Config(KBA k, float a, float e, float t=10, bool b=true) :
				BConfig(b), kba(k), angle(a), epsilon(e), timeout(t) {}
		};

		Align(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
		static void parse(Align::Config& cfg, ParameterToken t, ValueToken v);
	};

	struct Travel : public Action {
		struct Config : public BConfig {
			KBA kba;
			float dist;
			float epsilon;
			float timeout;

			Config(KBA k, float d, float e, float t=10, bool b=true) :
				BConfig(b), kba(k), dist(d), epsilon(e), timeout(t) {}
		};

		Travel(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
		static void parse(Travel::Config& cfg, ParameterToken t, ValueToken v);
	};

	struct ColorSort : public Action {
		struct Config : public BConfig {
			float timeout;

			Config(float t=5, bool b=true) :
				BConfig(b), timeout(t) {}
		};

		ColorSort(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
		static void parse(ColorSort::Config& cfg, ParameterToken t, ValueToken v);
	};

	typedef std::pair<std::string, std::queue<std::shared_ptr<Action>>> Routine;

	inline Routine* active_routine = nullptr;
	inline std::vector<std::shared_ptr<Routine>> routines; // <Name (of routine), Routine>
	inline std::queue<std::shared_ptr<Action>> actions_queue;
	inline std::vector<std::shared_ptr<Action>> actions_running;

	void select_routine(int index);
	void select_routine(std::string name);
	void initialize_actions_queue();
	void start_next_action();
	ActionsBlockingStatus run_action_ticks();

	extern std::string routines_directory;
	inline Align::Config def_align_cfg(KBA(), 0.0, 0.0);
	inline Travel::Config def_travel_cfg(KBA(), 0.0, 0.0);

	void load_routine_files();
	void parse_routine_file(std::filesystem::path path);
	template <typename T, typename G>
	G parse_parameter_token(
		G& cfg,
		std::vector<std::string> parameter_tokens,
		std::function<void(G&, ParameterToken, ValueToken)> parser_function
	);
}