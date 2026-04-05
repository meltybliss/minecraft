#pragma once
#include <stdint.h>
#include "world.h"
#include <memory>

struct ChunkPriority {//comparator
	int32_t plrCx;
	int32_t plrCz;

	bool operator()(const std::shared_ptr<Chunk>& a, const std::shared_ptr<Chunk>& b) const;


};