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

	typedef std::string ParameterToken, ValueToken;

	struct KBA {
		float k = 0;
		float b = 0;
		float a = 0;

		float output(float t) {
			return std::min(std::max(t*k, a), b);
		}

		static KBA parse(ValueToken v);
	};

        struct KBAIterable {
                KBA kba;
                float epsilon = 0.0;
                float timeout = 0.0;

                KBAIterable(KBA k, float e, float t) : kba(k), epsilon(e), timeout(t) {}
                virtual ~KBAIterable() = default;
        };

        struct Toggleable {
                bool toggling = false;
                float timeout = 0.0;

                Toggleable(bool tog, float t) : toggling(tog), timeout(t) {}
                virtual ~Toggleable() = default;

                static ActionRunStatus process_toggle(bool global_toggle_state);
        };

	struct BConfig {
		bool blocking = true;

		BConfig(bool b) : blocking(b) {}
		virtual ~BConfig() = default;
	};

	struct Action {
		float time = 0.0;

		std::shared_ptr<BConfig> c;
                Action(std::shared_ptr<BConfig> c) : c(std::move(c)) {}
		virtual ~Action() = default;
		virtual ActionRunStatus run_tick() = 0;
                virtual void start() = 0;
	};

	struct Align : Action {
		struct Config : BConfig, KBAIterable {
			float angle = 0.0;

			Config(KBA k, float a, float e, float t, bool b) :
				BConfig(b), angle(a), KBAIterable(k, e, t) {}
		};

		Align(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
                void start() override;

		static void parse(Config& cfg, ParameterToken t, ValueToken v);
                static void parse_cleanup(Config& cfg);
	};

	struct Travel : Action {
		struct Config : BConfig, KBAIterable {
			float dist = 0.0;

			Config(KBA k, float d, float e, float t, bool b) :
				BConfig(b), dist(d), KBAIterable(k, e, t) {}
		};

                float accum_dist = 0.0;

		Travel(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
                void start() override;

		static void parse(Config& cfg, ParameterToken t, ValueToken v);
                static void parse_cleanup(Config& cfg);
	};

	struct ColorSort : Action {
		struct Config : BConfig, Toggleable {
			Properties::ColorSortColor color = Properties::COLOR_SORT_RED;

			Config(Properties::ColorSortColor c, float t, bool tog, bool b) :
				BConfig(b), color(c), Toggleable(tog, t) {}
		};

                static inline bool toggle_state = false;

		ColorSort(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
                void start() override;

		static void parse(Config& cfg, ParameterToken t, ValueToken v);
                static void parse_cleanup(Config& cfg);
	};

	struct Intake : Action {
		struct Config : BConfig, Toggleable {
			Properties::IntakeMode intake_mode;

			Config(Properties::IntakeMode i_m, float t, bool tog, bool b) :
				BConfig(b), intake_mode(i_m), Toggleable(tog, t) {}
		};

                static inline bool toggle_state = false;

		Intake(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
                void start() override;

		static void parse(Config& cfg, ParameterToken t, ValueToken v);
                static void parse_cleanup(Config& cfg);
	};

        struct Hood : Action {
		struct Config : BConfig, Toggleable {
			Config(float t, bool tog, bool b) :
				BConfig(b), Toggleable(tog, t) {}
		};

                static inline bool toggle_state = false;

		Hood(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
                void start() override;

		static void parse(Config& cfg, ParameterToken t, ValueToken v);
                static void parse_cleanup(Config& cfg);
	};

        struct Unloader : Action {
		struct Config : BConfig, Toggleable {
			Config(float t, bool tog, bool b) :
				BConfig(b), Toggleable(tog, t) {}
		};

                static inline bool toggle_state = false;

		Unloader(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
                void start() override;

		static void parse(Config& cfg, ParameterToken t, ValueToken v);
                static void parse_cleanup(Config& cfg);
	};

        struct Descore : Action {
		struct Config : BConfig, Toggleable {
			Config(float t, bool tog, bool b) :
				BConfig(b), Toggleable(tog, t) {}
		};

                static inline bool toggle_state = false;

		Descore(Config c) : Action(std::make_shared<Config>(c)) {}
		ActionRunStatus run_tick() override;
                void start() override;

		static void parse(Config& cfg, ParameterToken t, ValueToken v);
                static void parse_cleanup(Config& cfg);
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
        extern std::string cache_directory;

	extern Align::Config def_align_cfg;
	extern Travel::Config def_travel_cfg;
	extern ColorSort::Config def_col_sort_cfg;
	extern Intake::Config def_intake_cfg;
        extern Hood::Config def_hood_cfg;
        extern Unloader::Config def_unloader_cfg;
        extern Descore::Config def_descore_cfg;

	void load_routine_files();
	void parse_routine_file(std::filesystem::path path);
	template <typename T, typename G>
	G parse_parameter_tokens(
		G& cfg,
		std::vector<std::string> parameter_tokens,
		std::function<void(G&, ParameterToken, ValueToken)> parser_function,
                std::function<void(G&)> parser_cleanup_function
	);
        void load_cache_file();
        void write_cache_file(std::string routine_name);
}