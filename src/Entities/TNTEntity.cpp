#include "Entities/TNTEntity.h"
#include "ExplosionService.h"
#include <cmath>


void TNTEntity::Tick(float dt) {
	timer -= dt;

	if (timer <= 0.0f) {

		ExplosionService::Explode(static_cast<int>(std::floor(pos.x)), static_cast<int>(std::floor(pos.y)),
			static_cast<int>(std::floor(pos.z)), 3);

		isDead = true;
	}

}

void TNTEntity::Render() {



}