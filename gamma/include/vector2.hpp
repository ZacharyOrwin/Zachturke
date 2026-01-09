#pragma once

#include <cmath>


struct Vector2 {
	
	float x = 0.0;
	float y = 0.0;

	Vector2 operator+(const Vector2 &other) {
		return Vector2{x + other.x, y + other.y};
	}

	Vector2 operator+=(const Vector2 &other) {
		return Vector2{x + other.x, y + other.y};
	}

	Vector2 operator-(const Vector2 &other) {
		return Vector2{x - other.x, y - other.y};
	}

	Vector2 operator-=(const Vector2 &other) {
		return Vector2{x - other.x, y - other.y};
	}

	Vector2 operator*(float scalar) {
		return Vector2{x * scalar, y * scalar};
	}

	Vector2 operator*=(float scalar) {
		return Vector2{x * scalar, y * scalar};
	}

	Vector2 operator/(float scalar) {
		return Vector2{x / scalar, y / scalar};
	}

	Vector2 operator/=(float scalar) {
		return Vector2{x / scalar, y / scalar};
	}

	Vector2 normalized() {
		float length = std::sqrt(x * x + y * y);

		if (length == 0) {
			return Vector2{0, 0};
		}

		return Vector2{x, y} / length;
	}
	
	float length() {
		return std::sqrt(std::pow(x, 2) + std::pow(y, 2));
	}

	float dot(const Vector2& other) {
		return x * other.x + y * other.y;
	}

	float cross(const Vector2& other) {
		return x * other.y - y * other.x;
	}
};