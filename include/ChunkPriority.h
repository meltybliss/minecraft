#pragma once
#include <stdint.h>
#include "world.h"

struct ChunkPriority {//comparator
	int32_t plrCx;
	int32_t plrCz;

	bool operator()(const Chunk* a, const Chunk* b) {//when it returns true, B is greater than A

		if (a->isEdited != b->isEdited) {
			return !a->isEdited && b->isEdited;
		}

		Vec2 offA = Vec2{ a->cx - plrCx, a->cz - plrCz };
		Vec2 offB = Vec2{ b->cx - plrCx, b->cz - plrCz };

		return gWorld->GetSpiralRank(offA.x, offA.z) > gWorld->GetSpiralRank(offB.x, offB.z);
	}


};