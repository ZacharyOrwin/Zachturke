#include "autonomous.hpp"
#include <fstream>


namespace Autonomous {

	std::string routines_directory = "/usd";

	Align::Config def_align_cfg(KBA(), 0.0, 0.0);
	Travel::Config def_travel_cfg(KBA(), 0.0, 0.0);
	ColorSort::Config def_col_sort_cfg(COLOR_SORT_RED);
	Intake::Config def_intake_cfg(Properties::INTAKE_TOP);


	void load_routine_files() {
		std::filesystem::path routines_path(routines_directory);
		std::filesystem::directory_iterator d_iter(routines_path);

		for (auto& dir : d_iter) {
			if (!dir.is_regular_file()) continue;

			std::filesystem::path p = dir.path();

			if (p.extension() == ".txt") {
				parse_routine_file(p);
			}
			
		}
	}


	/*

	Routine text files will generally be formatted as such:

	```txt
	ALIGN : KBA 2,4,6 ANG 90 EPS 0.0001 TIME 10
	TRAVEL : KBA 3,5,7 DIST 10 EPS 0.01 TIME 5
	This is a comment.
	ALIGN : ANG 135 TIME 10 KBA 2,4,8 EPS 0.0001 BLK 1
	TRAVEL : DIST 5
	```

	Lines can either contain a comment, or a definition token (ALIGN, TRAVEL)
	followed by parameter tokens

	Notice that in the 4th line, the order of parameter tokens doesn't matter;
	as long as the correct value token proceed each then it is valid.
	
	Also notice, the 5th line is an example of defaultation: any parameter tokens
	not defined will use the last parameter tokens from a previous call of the
	same definition token (in this case, from line 2).

	Some actions have default values for some of their actions already. For
	example, all actions by default block, and ALIGN and TRAVEL have timeouts
	of 10 seconds. Note, defaultation WILL override these.

	*/
	void parse_routine_file(std::filesystem::path path) {
		std::string name = path.stem();
		std::ifstream f(path);

		if (!f.is_open()) return;

		Routine rt;
		rt.first = name;
		
		for (std::string line; std::getline(f, line); ) {
			std::vector<std::string> tokens;
			std::istringstream tokens_stream(line);

			for (std::string token; std::getline(tokens_stream, token, ' '); ) {
				tokens.push_back(token);
			}

			std::string def = tokens.at(0);
			std::vector<std::string> param_tokens(tokens.begin() + 2, tokens.end());
			std::shared_ptr<Action> act;

			if (def == "ALIGN") {
				act = std::make_shared<Align>(
					parse_parameter_tokens<Align, Align::Config>(
						def_align_cfg,
						param_tokens,
						Align::parse
					)
				);

			} else if (def == "TRAVEL") {
				act = std::make_shared<Travel>(
					parse_parameter_tokens<Travel, Travel::Config>(
						def_travel_cfg,
						param_tokens,
						Travel::parse
					)
				);

			} else if (def == "COLSORT") {
				act = std::make_shared<ColorSort>(
					parse_parameter_tokens<ColorSort, ColorSort::Config>(
						def_col_sort_cfg,
						param_tokens,
						ColorSort::parse
					)
				);

			} else if (def == "INTAKE") {
				act = std::make_shared<Intake>(
					parse_parameter_tokens<Intake, Intake::Config>(
						def_intake_cfg,
						param_tokens,
						Intake::parse
					)
				);

			} else { continue; }

			rt.second.push(act);
		}

		routines.push_back(std::make_shared<Routine>(rt));
	}


	template <typename T, typename G>
	G parse_parameter_tokens(
		G& cfg,
		std::vector<std::string> parameter_tokens,
		std::function<void(G&, ParameterToken, ValueToken)> parser_function)
	{
		for (auto it = parameter_tokens.begin(); it == parameter_tokens.end() - 1; it += 2) {
			ParameterToken t = *it;
			ValueToken v = *(it+1);

			parser_function(cfg, t, v);
		}

		return cfg;
	}


	KBA KBA::parse(ValueToken v) {
		std::vector<float> kba_args;
		std::istringstream v_stream(v);

		for (std::string arg; std::getline(v_stream, arg, ','); ) {
			kba_args.push_back(std::stof(arg));
		}

		return KBA {
			.k = kba_args.at(0),
			.b = kba_args.at(1),
			.a = kba_args.at(2)
		};
	}


	void Align::parse(Align::Config& cfg, ParameterToken t, ValueToken v) {
		if (t == "KBA") {
			cfg.kba = KBA::parse(v);

		} else if (t == "ANG") {
			cfg.angle = std::stof(v);

		} else if (t == "EPS") {
			cfg.epsilon = std::stof(v);

		} else if (t == "TIME") {
			cfg.timeout = std::stof(v);

		} else if (t == "BLK") {
			cfg.blocking = std::stoi(v);
		}
	}


	void Travel::parse(Travel::Config& cfg, ParameterToken t, ValueToken v) {
		if (t == "KBA") {
			cfg.kba = KBA::parse(v);

		} else if (t == "DIST") {
			cfg.dist = std::stof(v);

		} else if (t == "EPS") {
			cfg.epsilon = std::stof(v);

		} else if (t == "TIME") {
			cfg.timeout = std::stof(v);

		} else if (t == "BLK") {
			cfg.blocking = std::stoi(v);
		}
	}


	void ColorSort::parse(ColorSort::Config& cfg, ParameterToken t, ValueToken v) {
		if (t == "COL") {
			if (v == "red") {
				cfg.color = COLOR_SORT_RED;
			} else if (v == "blue") {
				cfg.color = COLOR_SORT_BLUE;
			}

		} else if (t == "TOG") {
			cfg.toggling = std::stoi(v);

		} else if (t == "TIME") {
			cfg.timeout = std::stof(v);

		} else if (t == "BLK") {
			cfg.blocking = std::stoi(v);
		}
	}


	void Intake::parse(Intake::Config& cfg, ParameterToken t, ValueToken v) {
		if (t == "MODE") {
			if (v == "top") {
				cfg.intake_mode = Properties::INTAKE_TOP;
			} else if (v == "bottom") {
				cfg.intake_mode = Properties::INTAKE_BOTTOM;
			} else if (v == "reverse") {
				cfg.intake_mode = Properties::INTAKE_REVERSE;
			}
			
		} else if (t == "TOG") {
			cfg.toggling = std::stoi(v);

		} else if (t == "TIME") {
			cfg.timeout = std::stof(v);

		} else if (t == "BLK") {
			cfg.blocking = std::stoi(v);
		}
	}
}