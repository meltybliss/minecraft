#pragma once
#include <cmath>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Vec3.h"
#include "Ray.h"
#include "Mat4.h"


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

	
	Ray GetRay(float u, float v) const {
		return { pos, Normalize(lowerLeftCorner + (horizontal * u) + (vertical * v) - pos) };
	}


	Mat4 GetViewMatrix() const {
		// 現在の座標(pos)から、向いている方向(pos + forward)を見る
		return LookAt(pos, pos + forward, up);
	}

	// 2. 投影行列 (Perspective) を取得
	Mat4 GetProjectionMatrix() const {
		float fov = 45.0f;           // 視野角
		float aspect = 16.0f / 9.0f; // アスペクト比 (実働時はウィンドウサイズから計算が理想)
		float nearZ = 0.1f;
		float farZ = 1000.0f;
		return Perspective(fov, aspect, nearZ, farZ);
	}

	// 3. UpdateVectors 内の計算を整理
	void UpdateVectors() {
		// 1. オイラー角（Yaw, Pitch）から前方ベクトル（Forward）を計算
		const float radYaw = yaw * 3.14159265f / 180.0f;
		const float radPitch = pitch * 3.14159265f / 180.0f;

		Vec3 f;
		f.x = std::cos(radYaw) * std::cos(radPitch);
		f.y = std::sin(radPitch);
		f.z = std::sin(radYaw) * std::cos(radPitch);
		forward = Normalize(f);

		// 2. Forwardから右方向（Right）と上方向（Up）を計算
		// 世界の上方向（0, 1, 0）との外積で右を出す
		Vec3 worldUp{ 0.0f, 1.0f, 0.0f };
		right = Normalize(Cross(forward, worldUp));
		up = Normalize(Cross(right, forward));

		// 3. レイキャスト用：視野角（FOV）に基づいたビューポートの計算
		// Perspective行列で使用するFOV（ここでは45度）と一致させる
		float fovDeg = 45.0f;
		float fovRad = fovDeg * 3.14159265f / 180.0f;

		// 焦点距離（数学上のスクリーンまでの距離）
		float focal_length = 1.0f;

		// 視野角からスクリーンの物理的な高さを逆算する
		// tan(θ/2) = (height/2) / focal_length
		float viewport_height = 2.0f * std::tan(fovRad * 0.5f) * focal_length;

		// アスペクト比（16:9）を掛けて幅を出す
		float aspect_ratio = 16.0f / 9.0f;
		float viewport_width = aspect_ratio * viewport_height;

		// 4. 3D空間内でのスクリーンの広がりベクトル
		// カメラの右・上ベクトルにサイズを掛ける
		horizontal = right * viewport_width;
		vertical = up * viewport_height;

		// 5. レイキャストの基準点：スクリーンの左下隅を計算
		// 「カメラ位置」から「正面へ進み」、「左へ半分戻り」、「下へ半分戻る」
		lowerLeftCorner = pos + (forward * focal_length) - (horizontal * 0.5f) - (vertical * 0.5f);
	}
};

void UpdateCameraMovement(GLFWwindow* window, Camera& cam, float deltaTime);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);