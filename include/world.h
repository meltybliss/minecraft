#pragma once
#include "chunk.h"
#include <memory>
#include <unordered_map>
#include <stdint.h>
#include <cmath>
class World {
public:
	World();

	void Tick();
	void render();

private:

	int RENDER_DISTANCE = 2;
	int UNLOAD_DISTANCE = 4;

	std::unordered_map<uint64_t, std::unique_ptr<Chunk>> Chunks;

	uint64_t GetChunkKey(int32_t cx, int32_t cz) const {
		return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32 | static_cast<uint32_t>(cz));
	}

};

extern World* gWorld;