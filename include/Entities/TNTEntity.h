#pragma once
#include "Entity.h"
#include "BlockRenderUtils.h"
#include "AABB.h"

class TNTEntity : public Entity {
public:

	TNTEntity(const Vec3& startPos, float startTimer);
	~TNTEntity();

	void Tick(float dt) override;
	void Render(GLuint program) override;

private:

	float timer = 0.0f;
	float gravity = -20.0f;

	std::vector<float> verts;
	unsigned int vao = 0;
	unsigned int vbo = 0;
	int vertexCount = 0;

	AABB GetAABBAt() const;
	bool IntersectsSolidBlock();

	bool shouldFlash() const;

};