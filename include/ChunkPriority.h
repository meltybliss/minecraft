#pragma once
#include <stdint.h>
#include <memory>

struct Chunk;

struct ChunkPriority {//comparator
	int32_t plrCx;
	int32_t plrCz;

	bool operator()(const std::shared_ptr<Chunk>& a, const std::shared_ptr<Chunk>& b) const;


};