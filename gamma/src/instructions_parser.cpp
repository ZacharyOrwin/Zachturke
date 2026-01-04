#include "autonomous.hpp"
#include <fstream>


namespace Autonomous {

	std::string routines_directory = "/usd";


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
	# This is a comment.
	ALIGN : ANG 135 TIME 10 KBA 2,4,8 EPS 0.0001 BLK 1
	```
	
	Notice that the order of definition tokens doesn't matter - as long as
	the correct value token proceed each then it is valid.

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
			std::vector<std::string> pass_tokens(tokens.begin() + 2, tokens.end());
			std::unique_ptr<Action> act;

			if (def == "ALIGN") {

				act = std::make_unique<Align>(Align::parse(pass_tokens));

			} else if (def == "TRAVEL") {

				act = std::make_unique<Travel>(Travel::parse(pass_tokens));

			} else if (def == "#") {

				continue;

			}

			rt.second.push(std::make_shared<Action>(std::move(act)));
		}

		routines.push_back(std::make_shared<Routine>(rt));
	}


	Align Align::parse(std::vector<std::string> tokens) {
		Align::Config cfg(KBA(), 0.0, 0.0);

		for (auto it = tokens.begin(); it == tokens.end() - 1; ++it) {
			std::string t = *it;
			std::string v = *(it+1);

			if (t == "KBA") {

				std::vector<float> kba_args;
				std::istringstream v_stream(v);

				for (std::string arg; std::getline(v_stream, arg, ','); ) {
					kba_args.push_back(std::stof(arg));
				}

				cfg.kba = KBA {
					.k = kba_args.at(0),
					.b = kba_args.at(1),
					.a = kba_args.at(2)
				};

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

		return Align(cfg);
	}


	Travel Travel::parse(std::vector<std::string> tokens) {
		Travel::Config cfg(KBA(), 0.0, 0.0);

		for (auto it = tokens.begin(); it == tokens.end(); it += 2) {
			std::string t = *it;
			std::string v = *(it+1);

			if (t == "KBA") {

				std::vector<float> kba_args;
				std::istringstream v_stream(v);

				for (std::string arg; std::getline(v_stream, arg, ','); ) {
					kba_args.push_back(std::stof(arg));
				}

				cfg.kba = KBA {
					.k = kba_args.at(0),
					.b = kba_args.at(1),
					.a = kba_args.at(2)
				};

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

		return Travel(cfg);
	}
}