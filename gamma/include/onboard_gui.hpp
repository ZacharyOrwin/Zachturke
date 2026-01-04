#pragma once

#include "liblvgl/lvgl.h"


namespace OnboardGUI {

	lv_obj_t* routines_list;

	void initialize();
	void define_layout();
	void load_routines_list();
}