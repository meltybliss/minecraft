#pragma once
#include "chunk.h"
#include "Rendering/camera.h"
#include <memory>
#include <unordered_map>
#include <stdint.h>
#include <array>
#include <cmath>
#include <algorithm>
#include "Math/HitResult.h"
#include "Rendering/ShaderInitUtils.h"
#include "Math/Mat4.h"
#include <deque>
#include <queue>
#include "Math/Vec2.h"
#include "ChunkPriority.h"
#include <climits>
#include "Rendering/GpuDeleteJob.h"
#include "Math/Frustum.h"
#include "Rendering/ChunkMeshBuilder.h"
#include "TerrainGenerator.h"
#include "CaveGenerator.h"
#include "Entities/TNTEntity.h"
#include "BlockPos.h"
#include "WaterStruct.h"

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
		
		for (const auto& d : waterProcQueue) {
			if (bx == d.pos.x && by == d.pos.y && bz == d.pos.z) return;
		}

		
		waterProcQueue.emplace_back(WaterData{{bx, by, bz}, std::make_shared<int>(WaterDef::initialWaterLevel)});
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
	
	std::deque<uint64_t> urgentLightQueue;
	std::deque<uint64_t> normalLightQueue;

	std::priority_queue<
		std::shared_ptr<Chunk>,
		std::vector<std::shared_ptr<Chunk>>,
		ChunkPriority
	> meshQueue;

	std::deque<GpuDeleteJob> gpuDeleteQueue;
	std::deque<uint64_t> unloadQueue;
	std::deque<WaterData> waterProcQueue;


	//std::deque<std::weak_ptr<Chunk>> meshQueue;
	 
	std::vector<Vec2> spiralOffsets;
	std::unordered_map<uint64_t, int> spiralRank;//spiralOffset“à‚Å‚Ìindex‚ð“¾‚é‚½‚ß‚Ì‚à‚Ì

	std::vector<std::unique_ptr<Entity>> entities;
	std::vector<std::unique_ptr<Entity>> pendingEntities;

	uint64_t GetChunkKey(int32_t cx, int32_t cz) const {
		return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32 | static_cast<uint32_t>(cz));
	}

	void WakeNearbyWater(int bx, int by, int bz);

	
	void MarkChunkMeshDirty(int32_t cx, int32_t cz);
	void MarkChunkLightDirty(int32_t cx, int32_t cz, bool urgent);
	
	void RebuildMeshQueue(int32_t curCx, int32_t curCz);
	void GatherUnloadCandidates(int32_t curCx, int32_t curCz);

	void ChunkGenerate(Chunk* c);

	void ProcessGenQueue();
	void ProcessLightQueue();
	void ProcessMeshQueue();
	void ProcessGpuDeletes();
	void ProcessUnloadQueue(int32_t curCx, int32_t curCz);
	void ProcessWaterQueue();
};

extern World* gWorld;