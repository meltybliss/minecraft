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

	unsigned int GetBlockGlobal(int wx, int wy, int wz);
	Chunk* GetChunkPtr(int cx, int cz);
private:

	int RENDER_DISTANCE = 2;
	int UNLOAD_DISTANCE = 4;

	std::unordered_map<uint64_t, std::unique_ptr<Chunk>> Chunks;

	uint64_t GetChunkKey(int32_t cx, int32_t cz) const {
		return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32 | static_cast<uint32_t>(cz));
	}

	int32_t MaxCX = 40;
	int32_t MaxCZ = 40;


	void MarkChunkDirty(int32_t cx, int32_t cz);
};

extern World* gWorld;