#include "onboard_gui.hpp"
#include "autonomous.hpp"


namespace OnboardGUI {

	lv_obj_t* routines_lv_list = lv_list_create(lv_screen_active());
	lv_obj_t* selected_routine_lv_label = lv_label_create(lv_screen_active());


	void initialize() {
		define_visuals();
		load_routines_list();
	}


	// Define layouts, then positioning, hierarchy, and style of widgets.
	void define_visuals() {}


	void load_routines_list() {
		for (auto& routine : Autonomous::routines) {
			lv_obj_t* btn = lv_list_add_button(routines_lv_list, nullptr, routine->first.c_str());
			lv_obj_add_event_cb(btn, on_routine_btn_cb, LV_EVENT_VALUE_CHANGED, nullptr);
		}

		select_routine(lv_list_get_button_text(routines_lv_list, lv_obj_get_child(routines_lv_list, 0)));
	}

	
	void on_routine_btn_cb(lv_event_t* e) {
		lv_obj_t* btn = lv_event_get_target_obj(e);

		select_routine(lv_list_get_button_text(routines_lv_list, btn));
	}


	void select_routine(std::string routine_name) {
		lv_label_set_text(selected_routine_lv_label, routine_name.c_str());
		Autonomous::select_routine(routine_name);
	}
}