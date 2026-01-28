#include "autonomous.hpp"
#include <fstream>
#include <stdio.h>


namespace Autonomous {

	std::string routines_directory = "/";
	std::string cache_directory = "/cache.cfg";

<<<<<<< HEAD
	Align::Config def_align_cfg(KBAT(), 0.0, 0.0, 5.0, true);
	Travel::Config def_travel_cfg(KBAT(), 0.0, 0.0, 5.0, true);
	Pursue::Config def_pursue_cfg(KBAT(), KBAT(), Vector2{}, 0.0, 5.0, true);
=======
	Align::Config def_align_cfg(KBA(), 0.0, 0.0, 5.0, true);
	Travel::Config def_travel_cfg(KBA(), 0.0, 0.0, 5.0, true);
>>>>>>> parent of a106aa1 (KBAT replacing KBA.)
	ColorSort::Config def_col_sort_cfg(Properties::COLOR_SORT_RED, 0.0, 5.0, false, true);
	Intake::Config def_intake_cfg(Properties::INTAKE_TOP, 5.0, false, true);
	Hood::Config def_hood_cfg(5.0, false, true);
        Unloader::Config def_unloader_cfg(5.0, false, true);
        Descore::Config def_descore_cfg(5.0, false, true);
	Park::Config def_park_cfg(0, 0.0, 5.0, false, true);


	void load_routine_files() {
		char* files = (char*)malloc(1024);
		pros::usd::list_files(routines_directory.c_str(), files, 1024);
		std::istringstream files_iss(files);

		for (std::string file_name; std::getline(files_iss, file_name); ) {
			std::filesystem::path p(file_name);

			if (p.extension() == ".rtn") {
				parse_routine_file(p);
			}
		}

		std::free(files);
	}


	/*

	Routine text files will generally be formatted as such:

	```.rtn
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
						Align::parse,
						Align::parse_cleanup
					)
				);

			} else if (def == "TRAVEL") {
				act = std::make_shared<Travel>(
					parse_parameter_tokens<Travel, Travel::Config>(
						def_travel_cfg,
						param_tokens,
						Travel::parse,
						Travel::parse_cleanup
					)
				);

			} else if (def == "PURSUE") {
				act = std::make_shared<Pursue>(
					parse_parameter_tokens<Pursue, Pursue::Config>(
						def_pursue_cfg,
						param_tokens,
						Pursue::parse,
						Pursue::parse_cleanup
					)
				);
			
			
			} else if (def == "COLSORT") {
				act = std::make_shared<ColorSort>(
					parse_parameter_tokens<ColorSort, ColorSort::Config>(
						def_col_sort_cfg,
						param_tokens,
						ColorSort::parse,
						ColorSort::parse_cleanup
					)
				);

			} else if (def == "INTAKE") {
				act = std::make_shared<Intake>(
					parse_parameter_tokens<Intake, Intake::Config>(
						def_intake_cfg,
						param_tokens,
						Intake::parse,
						Intake::parse_cleanup
					)
				);

			} else if (def == "HOOD") {
				act = std::make_shared<Hood>(
					parse_parameter_tokens<Hood, Hood::Config>(
						def_hood_cfg,
						param_tokens,
						Hood::parse,
						Hood::parse_cleanup
					)
				);

			} else if (def == "UNLOADER") {
				act = std::make_shared<Unloader>(
					parse_parameter_tokens<Unloader, Unloader::Config>(
						def_unloader_cfg,
						param_tokens,
						Unloader::parse,
						Unloader::parse_cleanup
					)
				);

			} else if (def == "DESCORE") {
				act = std::make_shared<Descore>(
					parse_parameter_tokens<Descore, Descore::Config>(
						def_descore_cfg,
						param_tokens,
						Descore::parse,
						Descore::parse_cleanup
					)
				);

			} else if (def == "PARK") {
				act = std::make_shared<Park>(
					parse_parameter_tokens<Park, Park::Config>(
						def_park_cfg,
						param_tokens,
						Park::parse,
						Park::parse_cleanup
					)
				);

			} else { continue; }

			rt.second.push(act);
		}

		routines.push_back(std::make_shared<Routine>(rt));
		f.close();
	}


	template <typename T, typename G>
	G parse_parameter_tokens(
		G& cfg,
		std::vector<std::string> parameter_tokens,
		std::function<void(G&, ParameterToken, ValueToken)> parser_function,
		std::function<void(G&)> parser_cleanup_function
	) {
		for (auto it = parameter_tokens.begin();
			it < parameter_tokens.end() && (it + 1) < parameter_tokens.end();
			it += 2
		) {
			ParameterToken t = *it;
			ValueToken v = *(it+1);

			parser_function(cfg, t, v);
		}

		parser_cleanup_function(cfg);

		return cfg;
	}


	void load_cache_file() {
		std::filesystem::path p(cache_directory);
		std::ifstream f(p);

		if (!f.is_open()) return;
		
		for (std::string line; std::getline(f, line); ) {
			std::vector<std::string> tokens;
			std::istringstream tokens_stream(line);

			for (std::string token; std::getline(tokens_stream, token, ' '); ) {
				tokens.push_back(token);
			}

			std::string def = tokens.at(0);
			std::string val = tokens.at(1);

			if (def == "ROUTINE") {
				// NOT guaranteed to select a routine.
				// If the previously selected routine is now deleted,
				// then it cannot be selected. Will be nullptr.
				std::replace(val.begin(), val.end(), '_', ' ');
				select_routine(val);
			}
		}

		f.close();
	}


	void write_cache_file(std::string routine_name) {
		std::filesystem::path p(cache_directory);
		std::ofstream f(p);

		if (!f.is_open()) return;

		std::replace(routine_name.begin(), routine_name.end(), ' ', '_');
		f << "ROUTINE " + routine_name;

		f.close();
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


	Vector2 vector2_parse(ValueToken v) {
		std::vector<float> vec_args;
		std::istringstream v_stream(v);

		for (std::string arg; std::getline(v_stream, arg, ','); ) {
			vec_args.push_back(std::stof(arg));
		}

		return Vector2 {
			vec_args.at(0),
			vec_args.at(1)
		};
	}


	void Align::parse(Config& cfg, ParameterToken t, ValueToken v) {
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


	void Travel::parse(Config& cfg, ParameterToken t, ValueToken v) {
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


	void Pursue::parse(Config& cfg, ParameterToken t, ValueToken v) {
		if (t == "TKBAT") {
			cfg.travel_kbat = KBAT::parse(v);

		} else if (t == "AKBAT") {
			cfg.align_kbat = KBAT::parse(v);

		} else if (t == "POS") {
			cfg.position = vector2_parse(v);

		} else if (t == "EPS") {
			cfg.epsilon = std::stof(v);

		} else if (t == "TIME") {
			cfg.timeout = std::stof(v);

		} else if (t == "BLK") {
			cfg.blocking = std::stoi(v);
		}
	}


	void ColorSort::parse(Config& cfg, ParameterToken t, ValueToken v) {
		if (t == "COL") {
			if (v == "red") {
				cfg.color = Properties::COLOR_SORT_RED;
			} else if (v == "blue") {
				cfg.color = Properties::COLOR_SORT_BLUE;
			}
		
		} else if (t == "CROT") {
			cfg.closing_rotations = std::stof(v);

		} else if (t == "TOG") {
			cfg.toggling = std::stoi(v);

		} else if (t == "TIME") {
			cfg.timeout = std::stof(v);

		} else if (t == "BLK") {
			cfg.blocking = std::stoi(v);
		}
	}


	void Intake::parse(Config& cfg, ParameterToken t, ValueToken v) {
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


	void Hood::parse(Config& cfg, ParameterToken t, ValueToken v) {
		if (t == "TOG") {
			cfg.toggling = std::stoi(v);

		} else if (t == "TIME") {
			cfg.timeout = std::stof(v);

		} else if (t == "BLK") {
			cfg.blocking = std::stoi(v);
		}
	}


	void Unloader::parse(Config& cfg, ParameterToken t, ValueToken v) {
		if (t == "TOG") {
			cfg.toggling = std::stoi(v);

		} else if (t == "TIME") {
			cfg.timeout = std::stof(v);

		} else if (t == "BLK") {
			cfg.blocking = std::stoi(v);
		}
	}


	void Descore::parse(Config& cfg, ParameterToken t, ValueToken v) {
		if (t == "TOG") {
			cfg.toggling = std::stoi(v);

		} else if (t == "TIME") {
			cfg.timeout = std::stof(v);

		} else if (t == "BLK") {
			cfg.blocking = std::stoi(v);
		}
	}


	void Park::parse(Config& cfg, ParameterToken t, ValueToken v) {
		if (t == "SPD") {
			cfg.speed = std::stoi(v);

		} else if (t == "TDIS") {
			cfg.trigger_dist = std::stof(v);

		} else if (t == "TOG") {
			cfg.toggling = std::stoi(v);

		} else if (t == "TIME") {
			cfg.timeout = std::stof(v);

		} else if (t == "BLK") {
			cfg.blocking = std::stoi(v);
		}
	}


	void Align::parse_cleanup(Config& cfg) {}


	void Travel::parse_cleanup(Config& cfg) {}


	void Pursue::parse_cleanup(Config& cfg) {}


	void ColorSort::parse_cleanup(Config& cfg) {
		cfg.blocking = !cfg.toggling && cfg.blocking;
	}


	void Intake::parse_cleanup(Config& cfg) {
		cfg.blocking = !cfg.toggling && cfg.blocking;
	}


	void Hood::parse_cleanup(Config& cfg) {
		cfg.blocking = !cfg.toggling && cfg.blocking;
	}


	void Unloader::parse_cleanup(Config& cfg) {
		cfg.blocking = !cfg.toggling && cfg.blocking;
	}


	void Descore::parse_cleanup(Config& cfg) {
		cfg.blocking = !cfg.toggling && cfg.blocking;
	}


	void Park::parse_cleanup(Config& cfg) {
		cfg.blocking = !cfg.toggling && cfg.blocking;
	}
}