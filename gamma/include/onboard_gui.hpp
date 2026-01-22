#pragma once

#include "liblvgl/lvgl.h"
#include <string>


namespace OnboardGUI {

	inline lv_obj_t* routines_lv_list;
	inline lv_obj_t* selected_routine_lv_label;
	
	inline lv_obj_t* queued_actions_lv_list;
	inline lv_obj_t* running_actions_lv_list;

	inline lv_obj_t* red_team_btn;
	inline lv_obj_t* no_team_btn;
	inline lv_obj_t* blue_team_btn;

	void initialize_selector();
	void define_selector_visuals();
	void load_routines_list();

	void on_routine_btn_cb(lv_event_t* e);

	void select_routine(std::string routine_name);
}