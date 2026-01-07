#pragma once

#include "liblvgl/lvgl.h"
#include <string>


namespace OnboardGUI {

	inline lv_obj_t* routines_lv_list;
	inline lv_obj_t* selected_routine_lv_label;
	
	inline lv_obj_t* queued_actions_lv_list;
	inline lv_obj_t* running_actions_lv_list;

	void initialize_selector();
	void initialize_running();
	void define_selector_visuals();
	void define_running_visuals();
	void load_routines_list();
	void running_actions_view_run_tick();

	void on_routine_btn_cb(lv_event_t* e);

	void select_routine(std::string routine_name);
}