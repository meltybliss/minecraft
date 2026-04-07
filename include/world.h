#pragma once
#include "chunk.h"
#include "camera.h"
#include <memory>
#include <unordered_map>
#include <stdint.h>
#include <array>
#include <cmath>
#include <algorithm>
#include "HitResult.h"
#include "ShaderInitUtils.h"
#include "Mat4.h"
#include <deque>
#include <queue>
#include "Vec2.h"
#include "ChunkPriority.h"
#include <climits>
#include "GpuDeleteJob.h"
#include "Frustum.h"
#include "ChunkMeshBuilder.h"
#include "TerrainGenerator.h"
#include "CaveGenerator.h"
#include "Entities/TNTEntity.h"
#include "BlockPos.h"


class World {
public:
	World();

	void Tick(float dt);
	void render(GLuint program);

	unsigned int GetBlockGlobal(int bx, int by, int bz);
	Chunk* GetChunkPtr(int cx, int cz);

	bool SetBlockByRay(Ray& ray, unsigned int block, float maxDist);

	bool SetBlockGlobalForPlr(int bx, int by, int bz, unsigned int block);
	bool SetBlockGlobalForProgram(int bx, int by, int bz, unsigned int block);

	void Ignite(int bx, int by, int bz, float timer,
				bool hasExplosionSource = false,
				int ex = 0, int ey = 0, int ez = 0);

	HitResult TraceRay(Ray& ray, float maxDist);

	uint32_t getWorldSeed() const { return worldSeed; }

	int GetSpiralRank(int offx, int offz) const {
		auto it = spiralRank.find(GetChunkKey(offx, offz));

		if (it != spiralRank.end()) {
			return it->second;
		}

		return INT_MAX;
	}

	void EnqueueGpuDelete(GLuint vao, GLuint vbo) {
		gpuDeleteQueue.push_back({ vao, vbo });
	}

	void EnqueueWaterProc(int bx, int by, int bz) {
		for (const auto& p : waterProcQueue) {
			if (bx == p.x && by == p.y && bz == p.z) return;
		}

		waterProcQueue.push_back(BlockPos{ bx, by, bz });
	}

	float RandomFuse() {
		std::uniform_real_distribution<float> dist(0.5, 1.5);
		return dist(TNTRng);
	}
	
private:

	uint32_t worldSeed;
	std::mt19937 TNTRng;

	int RENDER_DISTANCE = 30;
	int UNLOAD_DISTANCE = 34;

	ChunkMeshBuilder meshBuilder;
	TerrainGenerator terrainGen;
	CaveGenerator caveGen;

	int32_t lastPlrChunkCx = INT_MAX;
	int32_t lastPlrChunkCz = INT_MAX;

	std::unordered_map<uint64_t, std::shared_ptr<Chunk>> Chunks;

	std::deque<std::weak_ptr<Chunk>> generationQueue;
	
	std::priority_queue<
		std::shared_ptr<Chunk>,
		std::vector<std::shared_ptr<Chunk>>,
		ChunkPriority
	> meshQueue;

	std::deque<GpuDeleteJob> gpuDeleteQueue;
	std::deque<uint64_t> unloadQueue;
	std::deque<BlockPos> waterProcQueue;


	//std::deque<std::weak_ptr<Chunk>> meshQueue;
	 
	std::vector<Vec2> spiralOffsets;
	std::unordered_map<uint64_t, int> spiralRank;//spiralOffset“à‚Å‚Ìindex‚ð“¾‚é‚½‚ß‚Ì‚à‚Ì

	std::vector<std::unique_ptr<Entity>> entities;

	uint64_t GetChunkKey(int32_t cx, int32_t cz) const {
		return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32 | static_cast<uint32_t>(cz));
	}

	
	void MarkChunkDirty(int32_t cx, int32_t cz);
	
	void RebuildMeshQueue(int32_t curCx, int32_t curCz);
	void GatherUnloadCandidates(int32_t curCx, int32_t curCz);

	void ChunkGenerate(Chunk* c);

	void ProcessGenQueue();
	void ProcessMeshQueue();
	void ProcessGpuDeletes();
	void ProcessUnloadQueue(int32_t curCx, int32_t curCz);
	void ProcessWaterQueue();
};

extern World* gWorld;