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


	void parse_routine_file(std::filesystem::path path) {
		std::string name = path.stem();
		std::ifstream f(path);

		if (!f.is_open()) return;

		Routine rt;
		rt.first = name;
		
		for (std::string line; std::getline(f, line); ) {
			std::vector<std::string> tokens;
			std::istringstream tokens_stream(line);

			for (std::string token; std::getline(tokens_stream, token); ) {
				tokens.push_back(token);
			}

			std::string def = tokens.at(0);
			std::unique_ptr<Action> act;

			if (def == "ALIGN") {
				act = std::make_unique<Align>(Align::parse(tokens));
			} else if (def == "TRAVEL") {
				act = std::make_unique<Travel>(Travel::parse(tokens));
			}

			rt.second.push(std::make_shared<Align>(std::move(act)));
		}

		routines.push_back(std::make_shared<Routine>(rt));
	}


	Align Align::parse(std::vector<std::string> tokens) {
		Align::Config cfg(KBA(), 0.0, 0.0, 0.0, true);

		return Align(cfg);
	}


	Travel Travel::parse(std::vector<std::string> tokens) {
		Travel::Config cfg(KBA(), 0.0, 0.0, 0.0, true);

		return Travel(cfg);
	}
}