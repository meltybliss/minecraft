#pragma once
#include "camera.h"
#include "Vec3.h"
#include "block.h"
#include "HitResult.h"
#include "Ray.h"

struct AABB {
	Vec3 min;
	Vec3 max;
};

class Player {
public:
	
	void Tick(float dt);
	void render();

	void OnMouseMove(float xoffset, float yoffset) {
		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f) pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;

		UpdateCamera();
	}

	Vec3& getPos() { return pos; }

	Camera& GetCamera() { return cam; }
	const Camera& GetCamera() const { return cam; }
private:
	Vec3 pos{0.0f, 150.0f, 0.0f};//‘«Œ³Šî€
	Vec3 vel;

	float yaw = -90;
	float pitch = 0;

	float radius = 0.3f;
	float height = 1.8f;
	float eyeHeight = 1.63;

	float walkSpeed = 5.0f;
	float gravity = -20.0f;
	float jmpPower = 10.0f;


	Camera cam;

	bool debugFly = false;
	bool grounded = false;

	BlockType selectedBlock = BlockType::TNT;

	HitResult lastHit;


	AABB GetAABBAt(const Vec3& p) const;
	bool IntersectsSolidBlock(const AABB& box);
	bool CanPlaceBlockAt(Vec3 blockPos) const;

	void UpdateCamera() {

		cam.pos = pos + Vec3{ 0.0f, eyeHeight, 0.0f };
		cam.yaw = yaw;
		cam.pitch = pitch;
		cam.UpdateVectors();
	}

	void UpdatePlrMovement(GLFWwindow* , float dt);
	void UpdateMouse();
};