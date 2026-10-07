#include "onboard_gui.hpp"
#include "autonomous.hpp"
#include "recorded_auton.hpp"
#include <algorithm>
#include <vector>

namespace OnboardGUI {
	namespace {
		std::vector<std::string> added_recordings;

		void add_recording_option_ui(const std::string& filename) {
			if (routines_lv_list == nullptr
				|| std::find(added_recordings.begin(), added_recordings.end(), filename)
					!= added_recordings.end()) {
				return;
			}

			for (const RecordedAuton::RecordingInfo& recording : RecordedAuton::list_recordings()) {
				if (recording.filename == filename) {
					lv_obj_t* btn = lv_list_add_button(
						routines_lv_list, nullptr, recording.label.c_str()
					);
					lv_obj_add_event_cb(btn, on_recording_btn_cb, LV_EVENT_CLICKED, nullptr);
					added_recordings.push_back(filename);
					return;
				}
			}
		}

		void refresh_recordings_cb(void*) {
			for (const RecordedAuton::RecordingInfo& recording : RecordedAuton::list_recordings()) {
				add_recording_option_ui(recording.filename);
			}
		}
	}

	void initialize_selector() {
		define_selector_visuals();
		load_routines_list();
	}


	void define_selector_visuals() {
		routines_lv_list = lv_list_create(lv_screen_active());
		selected_routine_lv_label = lv_label_create(lv_screen_active());

		lv_obj_set_x(selected_routine_lv_label, 200);
	}


	void load_routines_list() {
		for (auto& routine : Autonomous::routines) {
			lv_obj_t* btn = lv_list_add_button(routines_lv_list, nullptr, routine->first.c_str());
			lv_obj_add_event_cb(btn, on_routine_btn_cb, LV_EVENT_CLICKED, nullptr);
		}

		for (const RecordedAuton::RecordingInfo& recording : RecordedAuton::list_recordings()) {
			add_recording_option_ui(recording.filename);
		}

		if (Autonomous::active_routine == nullptr) {
			if (!RecordedAuton::selected_recording_file.empty()) {
				for (const RecordedAuton::RecordingInfo& recording : RecordedAuton::list_recordings()) {
					if (recording.filename == RecordedAuton::selected_recording_file) {
						lv_label_set_text(selected_routine_lv_label, recording.label.c_str());
						return;
					}
				}
			}

			if (!Autonomous::routines.empty()) {
				select_routine(Autonomous::routines.front()->first);
			} else {
				const std::vector<RecordedAuton::RecordingInfo> recordings =
					RecordedAuton::list_recordings();
				if (!recordings.empty()) {
					select_recording(recordings.front().filename);
				} else {
					lv_label_set_text(selected_routine_lv_label, "No autonomous routines");
				}
			}
		} else {
			lv_label_set_text(selected_routine_lv_label, Autonomous::active_routine->first.c_str());
		}
	}

	
	void on_routine_btn_cb(lv_event_t* e) {
		lv_obj_t* btn = lv_event_get_current_target_obj(e);

		select_routine(lv_list_get_button_text(routines_lv_list, btn));
	}

	void refresh_recordings() {
		const lv_result_t result = lv_async_call(refresh_recordings_cb, nullptr);
		if (result != LV_RESULT_OK) {
			std::printf("Could not schedule autonomous selector refresh.\n");
		}
	}

	void on_recording_btn_cb(lv_event_t* e) {
		lv_obj_t* btn = lv_event_get_current_target_obj(e);
		const char* label = lv_list_get_button_text(routines_lv_list, btn);
		for (const RecordedAuton::RecordingInfo& recording : RecordedAuton::list_recordings()) {
			if (recording.label == label) {
				select_recording(recording.filename);
				return;
			}
		}
	}

	void select_routine(std::string routine_name) {
		lv_label_set_text(selected_routine_lv_label, routine_name.c_str());
		Autonomous::select_routine(routine_name);
		Autonomous::write_cache_file(routine_name);
	}

	void select_recording(const std::string& filename) {
		for (const RecordedAuton::RecordingInfo& recording : RecordedAuton::list_recordings()) {
			if (recording.filename == filename) {
				lv_label_set_text(selected_routine_lv_label, recording.label.c_str());
				RecordedAuton::select_recording(filename);
				return;
			}
		}
	}
}