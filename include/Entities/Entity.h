#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Math/Vec3.h"

class Entity {
public:

	Entity(const Vec3& startPos) : pos(startPos) {}
	virtual ~Entity() = default;
	virtual void Tick(float dt) = 0;
	virtual void Render(GLuint program) = 0;

	bool IsDead() const { return isDead; }

	void setVel(Vec3 vel) { this->vel = vel; }
	

protected:

	Vec3 pos;
	Vec3 vel;
	bool isDead = false;

};