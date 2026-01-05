#pragma once

#include "liblvgl/lvgl.h"
#include <string>


namespace OnboardGUI {

	extern lv_obj_t* routines_lv_list;
	extern lv_obj_t* selected_routine_lv_label;

	void initialize();
	void define_visuals();
	void load_routines_list();

	void on_routine_btn_cb(lv_event_t* e);

	void select_routine(std::string routine_name);
}