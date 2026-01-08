#pragma once

#include "api.h"
#include "properties.hpp"
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

	enum ColorSortColor {
		COLOR_SORT_RED,
		COLOR_SORT_BLUE
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

        struct Toggleable {
                bool toggled;

                ActionRunStatus process_toggle(bool& global_toggle_state);
        };

	struct BConfig {
		bool blocking = true;

		BConfig(bool b) : blocking(b) {}
		virtual ~BConfig() = default;
	};

	struct Action {
		float time = 0;

		std::shared_ptr<BConfig> c;
                Action(std::shared_ptr<BConfig> c) : c(std::move(c)) {}
		virtual ~Action() = default;
		virtual ActionRunStatus run_tick() = 0;
	};

	struct Align : Action {
		struct Config : BConfig {
			KBA kba;
			float angle;
			float epsilon;
			float timeout;

			Config(KBA k, float a, float e, float t, bool b) :
				BConfig(b), kba(k), angle(a), epsilon(e), timeout(t) {}
		};

		Align(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
		static void parse(Align::Config& cfg, ParameterToken t, ValueToken v);
	};

	struct Travel : Action {
		struct Config : BConfig {
			KBA kba;
			float dist;
			float epsilon;
			float timeout;

			Config(KBA k, float d, float e, float t, bool b) :
				BConfig(b), kba(k), dist(d), epsilon(e), timeout(t) {}
		};

                float accum_dist;

		Travel(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
		static void parse(Travel::Config& cfg, ParameterToken t, ValueToken v);
	};

	struct ColorSort : Action, Toggleable {
		struct Config : BConfig {
			ColorSortColor color;
			bool toggling;
			float timeout;

			Config(ColorSortColor c, float t, bool tog, bool b) :
				BConfig(b), color(c), toggling(tog), timeout(t) {}
		};

                static inline bool col_sort_toggle_state;

		ColorSort(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
		static void parse(ColorSort::Config& cfg, ParameterToken t, ValueToken v);
	};

	struct Intake : Action, Toggleable {
		struct Config : BConfig {
			Properties::IntakeMode intake_mode;
			bool toggling;
			float timeout; 

			Config(Properties::IntakeMode i_m, float t, bool tog, bool b) :
				BConfig(b), intake_mode(i_m), toggling(tog), timeout(t) {}
		};

                static inline bool intake_toggle_state;

		Intake(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
		static void parse(Intake::Config& cfg, ParameterToken t, ValueToken v);
	};

        std::string get_action_type(std::shared_ptr<Action>& action);

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
	extern Align::Config def_align_cfg;
	extern Travel::Config def_travel_cfg;
	extern ColorSort::Config def_col_sort_cfg;
	extern Intake::Config def_intake_cfg;

	void load_routine_files();
	void parse_routine_file(std::filesystem::path path);
	template <typename T, typename G>
	G parse_parameter_tokens(
		G& cfg,
		std::vector<std::string> parameter_tokens,
		std::function<void(G&, ParameterToken, ValueToken)> parser_function
	);
}