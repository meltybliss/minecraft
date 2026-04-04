#pragma once
#include "chunk.h"
#include "camera.h"
#include <memory>
#include <unordered_map>
#include <stdint.h>
#include <cmath>
#include "HitResult.h"
#include "ShaderInitUtils.h"
#include "Mat4.h"
#include <deque>


class World {
public:
	World();

	void Tick();
	void render();

	unsigned int GetBlockGlobal(int bx, int by, int bz);
	Chunk* GetChunkPtr(int cx, int cz);

	bool SetBlockByRay(Ray& ray, unsigned int block, float maxDist);
	bool SetBlockGlobal(int bx, int by, int bz, unsigned int block);

	HitResult TraceRay(Ray& ray, float maxDist);

	uint32_t getWorldSeed() const { return worldSeed; }

private:

	uint32_t worldSeed;

	int RENDER_DISTANCE = 30;
	int UNLOAD_DISTANCE = 34;

	std::deque<Chunk*> generationQueue;
	std::deque<Chunk*> meshQueue;
	 

	std::unordered_map<uint64_t, std::unique_ptr<Chunk>> Chunks;

	uint64_t GetChunkKey(int32_t cx, int32_t cz) const {
		return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32 | static_cast<uint32_t>(cz));
	}


	void MarkChunkDirty(int32_t cx, int32_t cz);
	


};

extern World* gWorld;