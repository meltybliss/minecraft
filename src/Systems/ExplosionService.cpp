#include "Systems/ExplosionService.h"
#include "World/world.h"


void ExplosionService::Explode(int bx, int by, int bz, int radius) {
	int r2 = radius * radius;

	for (int dy = -radius; dy <= radius; dy++) {
		for (int dz = -radius; dz <= radius; dz++) {
			for (int dx = -radius; dx <= radius; dx++) {
				int dist2 = dx * dx + dy * dy + dz * dz;
				if (dist2 > r2) continue;

				int x = bx + dx;
				int y = by + dy;
				int z = bz + dz;

				if (gWorld->GetBlockGlobal(x, y, z) == (unsigned int)BlockType::TNT) {
					
					gWorld->Ignite(x, y, z, gWorld->RandomFuse(), true, bx, by, bz);
				}
				else {
					gWorld->SetBlockGlobalForProgram(x, y, z, 0);
				}
				
			}
		}
	}

}