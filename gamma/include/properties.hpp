#pragma once


namespace Properties {
        enum IntakeMode {
                INTAKE_OFF,
		INTAKE_REVERSE,
		INTAKE_TOP,
		INTAKE_BOTTOM
	};

        inline constexpr int TICK_DELAY_MSEC = 5;
        inline IntakeMode intake_mode = INTAKE_TOP;
        inline bool col_sort_active;
        inline bool intake_active;
}