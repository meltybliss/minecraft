#pragma once
#include <cmath>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Vec3.h"
#include "Ray.h"


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
	Vec3 pos{ 0.0f, 100.0f, 0.0f };

	float yaw = -90.0f;
	float pitch = 0.0f;

	Vec3 forward{ 0.0f, 0.0f, -1.0f };
	Vec3 right{ 1.0f, 0.0f, 0.0f };
	Vec3 up{ 0.0f, 1.0f, 0.0f };

	Vec3 lowerLeftCorner;
	Vec3 horizontal;
	Vec3 vertical;

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

		//raycast
		float aspect_ratio = 16.0f / 9.0f; // 画面比率
		float viewport_height = 2.0f;     // 仮想視界の高さ
		float viewport_width = aspect_ratio * viewport_height;
		float focal_length = 1.0f;        // 焦点距離

		// カメラが向いている方向に基づいた水平・垂直ベクトル
		horizontal = right * viewport_width;
		vertical = up * viewport_height;

		// スクリーンの左下隅の座標を計算
		// pos - (横/2) - (縦/2) + (前方向 * 距離)
		lowerLeftCorner = pos - (horizontal * 0.5f) - (vertical * 0.5f) + (forward * focal_length);
	}

	Ray GetRay(float u, float v) const {
		return { pos, Normalize(lowerLeftCorner + (horizontal * u) + (vertical * v) - pos) };
	}
};

void UpdateCameraMovement(GLFWwindow* window, Camera& cam, float deltaTime);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);