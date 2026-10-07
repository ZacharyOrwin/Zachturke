#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace RecordedAuton {

	enum Button : std::uint16_t {
		BUTTON_L1 = 1u << 0,
		BUTTON_L2 = 1u << 1,
		BUTTON_R1 = 1u << 2,
		BUTTON_R2 = 1u << 3,
		BUTTON_UP = 1u << 4,
		BUTTON_DOWN = 1u << 5,
		BUTTON_LEFT = 1u << 6,
		BUTTON_RIGHT = 1u << 7,
		BUTTON_X = 1u << 8,
		BUTTON_B = 1u << 9,
		BUTTON_Y = 1u << 10,
		BUTTON_A = 1u << 11
	};

	struct RecordingInfo {
		std::string filename;
		std::string label;
	};

	inline std::string selected_recording_file;

	std::vector<RecordingInfo> list_recordings();
	void update_driver_recording();
	void stop_recording();
	bool is_recording();
	bool restore_selected_recording(const std::string& filename);
	bool select_recording(const std::string& filename);
	void clear_selected_recording();
	void replay_selected_recording();

}
