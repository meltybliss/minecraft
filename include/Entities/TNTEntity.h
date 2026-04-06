#pragma once
#include "Entity.h"
#include "BlockRenderUtils.h"

class TNTEntity : public Entity {
public:

	TNTEntity(const Vec3& startPos, float startTimer)
		: Entity(startPos), timer(startTimer) {}

	void Tick(float dt) override;
	void Render() override;

private:

	float timer = 0.0f;
	std::vector<float> verts;

};