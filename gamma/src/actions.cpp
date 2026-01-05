#include "autonomous.hpp"
#include "bot_connections.hpp"
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

	
	ActionRunStatus ColorSort::run_tick() {
		Config* cfg = static_cast<Config*>(c.get());

		time += Properties::TICK_DELAY_MSEC/1000.0;

		if (cfg->toggling && !Properties::col_sort_active && !toggled) {
			Properties::col_sort_active = true;
			toggled = true;

		} else if (cfg->toggling && Properties::col_sort_active && !toggled) {
			Properties::col_sort_active = false;
			return ACTION_RUN_COMPLETE;

		// Exit point here in if-block for toggling.
		} else if (toggled) {

		// The last condition for non-toggling actions or switched off via other action.
		} else if (time > cfg->timeout || !Properties::col_sort_active) {
			return ACTION_RUN_COMPLETE;
		}

		double hue = BotConnections::vis_sens.get_hue();

		bool is_red  = (hue >= 350 || hue <= 10);
		bool is_blue = (hue >= 200 && hue <= 220);

		// Backdoor DEFINITE WIP
		if (is_red && cfg->color == COLOR_SORT_BLUE || is_blue && cfg->color == COLOR_SORT_RED) {
			BotConnections::flap.set_value(true);
		} else {
			BotConnections::flap.set_value(false);
		}
		
		return ACTION_RUN_ONGOING;
	}


	ActionRunStatus Intake::run_tick() {
		Config* cfg = static_cast<Config*>(c.get());

		Properties::intake_mode = cfg->intake_mode;
		time += Properties::TICK_DELAY_MSEC/1000.0;

		if (cfg->toggling && !Properties::intake_active && !toggled) {
			Properties::intake_active = true;
			toggled = true;

		} else if (cfg->toggling && Properties::intake_active && !toggled) {
			Properties::intake_active = false;

			BotConnections::intake_A.brake();
			BotConnections::intake_B.brake();
			BotConnections::intake_C.brake();
			BotConnections::flap.set_value(false);

			return ACTION_RUN_COMPLETE;

		// Exit point here in if-block for toggling.
		} else if (toggled) {

		// The last condition for non-toggling actions or switched off via other action.
		} else if (time > cfg->timeout || !Properties::intake_active) {
			BotConnections::intake_A.brake();
			BotConnections::intake_B.brake();
			BotConnections::intake_C.brake();
			BotConnections::flap.set_value(false);

			return ACTION_RUN_COMPLETE;
		}

		switch (Properties::intake_mode) {
			case Properties::INTAKE_REVERSE:
				BotConnections::intake_A.move(-127);
				BotConnections::intake_B.move(-127);
				break;

			case Properties::INTAKE_TOP:
				BotConnections::intake_A.move(127);
				BotConnections::intake_B.move(127);
				BotConnections::intake_C.move(127);
				break;
	
			case Properties::INTAKE_BOTTOM:
				BotConnections::intake_A.move(127);
				BotConnections::intake_B.move(127);
				BotConnections::intake_C.move(-127);
				break;
	
			case Properties::INTAKE_OFF:
				BotConnections::intake_A.brake();
				BotConnections::intake_B.brake();
				BotConnections::intake_C.brake();
				BotConnections::flap.set_value(false);
				break;
		}

		return ACTION_RUN_ONGOING;
	}
}