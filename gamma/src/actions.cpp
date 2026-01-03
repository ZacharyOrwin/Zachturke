#include "autonomous.hpp"
#include "properties.hpp"


namespace Autonomous {

	ActionRunStatus Align::run_tick() {
                Config* cfg = static_cast<Config*>(c.get());

		time += Properties::TICK_DELAY_MSEC/1000.0;

		if (time > cfg->timeout) return ACTION_RUN_COMPLETE;

		return ACTION_RUN_ONGOING;
	}


	ActionRunStatus Travel::run_tick() {
                Config* cfg = static_cast<Config*>(c.get());

		time += Properties::TICK_DELAY_MSEC/1000.0;

		if (time > cfg->timeout) return ACTION_RUN_COMPLETE;

		return ACTION_RUN_ONGOING;
	}
}