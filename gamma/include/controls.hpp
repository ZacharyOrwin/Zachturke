#pragma once

#include "api.h"
#include <cstdint>


namespace Controls {
	
	void processDrive();
	void processLondon();
	void processLondonButtons(std::uint16_t buttons);
}