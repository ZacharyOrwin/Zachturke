#pragma once

#include <numbers>


namespace XMath {
        
        template <typename T> int sgn(T val) {
                return (T(0) < val) - (val < T(0));
        }

        float rad(float degrees) {
                return degrees * std::numbers::pi / 180.0;
        }

        float rpmToRads(float rpm) {
                return rpm * std::numbers::pi / 30.0;
        }
}