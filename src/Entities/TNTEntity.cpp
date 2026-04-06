#include "Entities/TNTEntity.h"
#include "ExplosionService.h"
#include <cmath>
#include "block.h"

void TNTEntity::Tick(float dt) {
	timer -= dt;

	if (timer <= 0.0f) {

		ExplosionService::Explode(static_cast<int>(std::floor(pos.x)), static_cast<int>(std::floor(pos.y)),
			static_cast<int>(std::floor(pos.z)), 5);

		isDead = true;
	}

}

void TNTEntity::Render() {

	BlockRenderUtils::AppendBlockCube(verts, pos.x, pos.y, pos.z, 
		(unsigned int)BlockType::TNT);

}