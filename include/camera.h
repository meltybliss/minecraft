#pragma once
#include <cmath>

struct Vec3 {
	float x, y, z;

	Vec3 operator+(const Vec3& other) const {
		return { x + other.x, y + other.y, z + other.z };
	}

	Vec3 operator-(const Vec3& other) const {
		return { x - other.x, y - other.y, z - other.z };
	}

	Vec3 operator*(const Vec3& other) const {
		return { x * other.x, y * other.y, z * other.z };
	}

	Vec3 operator*(float s) const {
		return { x * s, y * s, z * s };
	}

	Vec3& operator+=(const Vec3& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vec3& operator-=(const Vec3& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
};


inline float Length(const Vec3& v) {
	return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}


inline Vec3 Normalize(const Vec3& v) {
	float len = Length(v);
	if (len == 0.0f) return { 0.0f, 0.0f, 0.0f };

	return { v.x / len, v.y / len, v.z / len };
}

inline Vec3 Cross(const Vec3& a, const Vec3& b) {
	return {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}


struct Camera {
	Vec3 pos{ 0.0f, 20.0f, 0.0f };

	float yaw = -90.0f;
	float pitch = 0.0f;

	Vec3 forward{ 0.0f, 0.0f, -1.0f };
	Vec3 right{ 1.0f, 0.0f, 0.0f };
	Vec3 up{ 0.0f, 1.0f, 0.0f };

	void UpdateVectors() {
		const float radYaw = yaw * 3.14159265f / 180.0f;
		const float radPitch = pitch * 3.14159265f / 180.0f;

		forward.x = std::cos(radYaw) * std::cos(radPitch);
		forward.y = std::sin(radPitch);
		forward.z = std::sin(radYaw) * std::cos(radPitch);
		forward = Normalize(forward);

		Vec3 worldUp{ 0.0f, 1.0f, 0.0f };
		right = Normalize(Cross(forward, worldUp));
		up = Normalize(Cross(right, forward));
	}


};