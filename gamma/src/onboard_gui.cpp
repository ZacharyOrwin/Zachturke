#include "onboard_gui.hpp"
#include "autonomous.hpp"
#include "recorded_auton.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace OnboardGUI {
	namespace {
		constexpr std::uint32_t COLOR_BACKGROUND = 0x080D16;
		constexpr std::uint32_t COLOR_PANEL = 0x101A2A;
		constexpr std::uint32_t COLOR_BUTTON = 0x16243A;
		constexpr std::uint32_t COLOR_BLUE = 0x168BFF;
		constexpr std::uint32_t COLOR_BLUE_PRESSED = 0x0D5FC0;
		constexpr std::uint32_t COLOR_BORDER = 0x243955;
		constexpr std::uint32_t COLOR_TEXT = 0xF2F6FC;
		constexpr std::uint32_t COLOR_MUTED = 0x8EA2BC;

		std::vector<std::string> added_recordings;

		void style_panel(lv_obj_t* panel) {
			lv_obj_set_style_bg_color(panel, lv_color_hex(COLOR_PANEL), LV_PART_MAIN);
			lv_obj_set_style_border_color(panel, lv_color_hex(COLOR_BORDER), LV_PART_MAIN);
			lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN);
			lv_obj_set_style_radius(panel, 12, LV_PART_MAIN);
			lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);
		}

		void style_routine_button(lv_obj_t* button) {
			lv_obj_set_style_bg_color(button, lv_color_hex(COLOR_BUTTON), LV_PART_MAIN);
			lv_obj_set_style_bg_color(
				button, lv_color_hex(COLOR_BLUE_PRESSED), LV_PART_MAIN | LV_STATE_PRESSED
			);
			lv_obj_set_style_border_color(button, lv_color_hex(COLOR_BORDER), LV_PART_MAIN);
			lv_obj_set_style_border_width(button, 1, LV_PART_MAIN);
			lv_obj_set_style_radius(button, 6, LV_PART_MAIN);
			lv_obj_set_style_text_color(button, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
			lv_obj_set_style_pad_left(button, 10, LV_PART_MAIN);
		}

		lv_obj_t* add_styled_routine_button(const char* name) {
			lv_obj_t* button = lv_list_add_button(routines_lv_list, nullptr, name);
			style_routine_button(button);
			return button;
		}

		void add_recording_option_ui(const std::string& filename) {
			if (routines_lv_list == nullptr
				|| std::find(added_recordings.begin(), added_recordings.end(), filename)
					!= added_recordings.end()) {
				return;
			}

			for (const RecordedAuton::RecordingInfo& recording : RecordedAuton::list_recordings()) {
				if (recording.filename == filename) {
					lv_obj_t* btn = add_styled_routine_button(recording.label.c_str());
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
		lv_obj_t* screen = lv_screen_active();
		lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_BACKGROUND), LV_PART_MAIN);
		lv_obj_set_style_border_width(screen, 0, LV_PART_MAIN);

		lv_obj_t* brand_accent = lv_obj_create(screen);
		lv_obj_set_pos(brand_accent, 16, 14);
		lv_obj_set_size(brand_accent, 4, 34);
		lv_obj_set_style_bg_color(brand_accent, lv_color_hex(COLOR_BLUE), LV_PART_MAIN);
		lv_obj_set_style_border_width(brand_accent, 0, LV_PART_MAIN);
		lv_obj_set_style_radius(brand_accent, 2, LV_PART_MAIN);

		lv_obj_t* team_label = lv_label_create(screen);
		lv_label_set_text(team_label, "SKG 4873S");
		lv_obj_set_pos(team_label, 29, 10);
		lv_obj_set_style_text_color(team_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

		lv_obj_t* subtitle = lv_label_create(screen);
		lv_label_set_text(subtitle, "AUTONOMOUS SELECTOR");
		lv_obj_set_pos(subtitle, 30, 32);
		lv_obj_set_style_text_color(subtitle, lv_color_hex(COLOR_MUTED), LV_PART_MAIN);

		lv_obj_t* routines_panel = lv_obj_create(screen);
		lv_obj_set_pos(routines_panel, 16, 62);
		lv_obj_set_size(routines_panel, 248, 160);
		style_panel(routines_panel);

		lv_obj_t* selected_panel = lv_obj_create(screen);
		lv_obj_set_pos(selected_panel, 276, 62);
		lv_obj_set_size(selected_panel, 188, 160);
		style_panel(selected_panel);

		lv_obj_t* routines_heading = lv_label_create(routines_panel);
		lv_label_set_text(routines_heading, "AVAILABLE ROUTINES");
		lv_obj_set_pos(routines_heading, 12, 10);
		lv_obj_set_style_text_color(routines_heading, lv_color_hex(COLOR_MUTED), LV_PART_MAIN);

		lv_obj_t* selected_heading = lv_label_create(selected_panel);
		lv_label_set_text(selected_heading, "ACTIVE ROUTINE");
		lv_obj_set_pos(selected_heading, 12, 10);
		lv_obj_set_style_text_color(selected_heading, lv_color_hex(COLOR_MUTED), LV_PART_MAIN);

		routines_lv_list = lv_list_create(routines_panel);
		lv_obj_set_pos(routines_lv_list, 10, 36);
		lv_obj_set_size(routines_lv_list, 228, 114);
		lv_obj_set_style_bg_opa(routines_lv_list, LV_OPA_TRANSP, LV_PART_MAIN);
		lv_obj_set_style_border_width(routines_lv_list, 0, LV_PART_MAIN);
		lv_obj_set_style_pad_all(routines_lv_list, 0, LV_PART_MAIN);
		lv_obj_set_style_pad_row(routines_lv_list, 5, LV_PART_MAIN);
		lv_obj_set_scrollbar_mode(routines_lv_list, LV_SCROLLBAR_MODE_OFF);

		selected_routine_lv_label = lv_label_create(selected_panel);
		lv_label_set_long_mode(selected_routine_lv_label, LV_LABEL_LONG_DOT);
		lv_obj_set_pos(selected_routine_lv_label, 24, 50);
		lv_obj_set_size(selected_routine_lv_label, 140, 38);
		lv_obj_set_style_text_color(
			selected_routine_lv_label, lv_color_hex(COLOR_BLUE), LV_PART_MAIN
		);

		lv_obj_t* status_accent = lv_obj_create(selected_panel);
		lv_obj_set_pos(status_accent, 12, 119);
		lv_obj_set_size(status_accent, 4, 18);
		lv_obj_set_style_bg_color(status_accent, lv_color_hex(COLOR_BLUE), LV_PART_MAIN);
		lv_obj_set_style_border_width(status_accent, 0, LV_PART_MAIN);
		lv_obj_set_style_radius(status_accent, 2, LV_PART_MAIN);

		lv_obj_t* status_label = lv_label_create(selected_panel);
		lv_label_set_text(status_label, "READY TO RUN");
		lv_obj_set_pos(status_label, 24, 120);
		lv_obj_set_style_text_color(status_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

		lv_obj_t* footer = lv_label_create(screen);
		lv_label_set_text(footer, "TAP A ROUTINE TO SELECT");
		lv_obj_set_pos(footer, 16, 226);
		lv_obj_set_style_text_color(footer, lv_color_hex(COLOR_MUTED), LV_PART_MAIN);
	}


	void load_routines_list() {
		for (auto& routine : Autonomous::routines) {
			lv_obj_t* button = add_styled_routine_button(routine->first.c_str());
			lv_obj_add_event_cb(button, on_routine_btn_cb, LV_EVENT_CLICKED, nullptr);
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