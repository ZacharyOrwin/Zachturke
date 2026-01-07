#include "onboard_gui.hpp"
#include "autonomous.hpp"


namespace OnboardGUI {

	void initialize_selector() {
		define_selector_visuals();
		load_routines_list();
	}


	void initialize_running() {
		lv_obj_clean(lv_screen_active());

		define_running_visuals();
	}


	void define_selector_visuals() {
		routines_lv_list = lv_list_create(lv_screen_active());
		selected_routine_lv_label = lv_label_create(lv_screen_active());

		lv_obj_set_x(selected_routine_lv_label, 200);
	}


	void define_running_visuals() {
		queued_actions_lv_list = lv_list_create(lv_screen_active());
		running_actions_lv_list = lv_list_create(lv_screen_active());

		lv_obj_set_x(running_actions_lv_list, 200);
	}


	void load_routines_list() {
		for (auto& routine : Autonomous::routines) {
			lv_obj_t* btn = lv_list_add_button(routines_lv_list, nullptr, routine->first.c_str());
			lv_obj_add_event_cb(btn, on_routine_btn_cb, LV_EVENT_CLICKED, nullptr);
		}

		select_routine(lv_list_get_button_text(routines_lv_list, lv_obj_get_child(routines_lv_list, 0)));
	}


	void running_actions_view_run_tick() {
		lv_obj_clean(queued_actions_lv_list);
		lv_obj_clean(running_actions_lv_list);

		std::queue<std::shared_ptr<Autonomous::Action>> queued_actions = Autonomous::actions_queue;

		while (!queued_actions.empty()) {
			lv_list_add_text(
				queued_actions_lv_list,
				Autonomous::get_action_type(queued_actions.front()).c_str()
			);
			queued_actions.pop();
		}

		for (auto& action : Autonomous::actions_running) {
			lv_list_add_text(
				running_actions_lv_list,
				Autonomous::get_action_type(action).c_str()
			);
		}
	}

	
	void on_routine_btn_cb(lv_event_t* e) {
		lv_obj_t* btn = lv_event_get_current_target_obj(e);

		select_routine(lv_list_get_button_text(routines_lv_list, btn));
	}


	void select_routine(std::string routine_name) {
		lv_label_set_text(selected_routine_lv_label, routine_name.c_str());
		Autonomous::select_routine(routine_name);
	}
}