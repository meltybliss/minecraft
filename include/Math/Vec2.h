#pragma once

struct Vec2 {
	int x, z;

	Vec2 operator+(const Vec2& other) const {
		return { x + other.x, z + other.z };
	}

	Vec2 operator-(const Vec2& other) const {
		return { x - other.x, z - other.z };
	}

	Vec2 operator*(const Vec2& other) const {
		return { x * other.x, z * other.z };
	}

	Vec2 operator*(int s) const {
		return { x * s, z * s };
	}

	Vec2& operator+=(const Vec2& other) {
		x += other.x;
		z += other.z;
		return *this;
	}

	Vec2& operator-=(const Vec2& other) {
		x -= other.x;
		z -= other.z;
		return *this;
	}

};