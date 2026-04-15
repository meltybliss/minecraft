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
#include <unordered_set>
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
#include "ChunkPool.h"




struct LightNode {
	int x, y, z;
	uint8_t skyLight = 0;
};

struct RemoveNode {
	int x, y, z;
	uint8_t oldLight;
};


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

	int RENDER_DISTANCE = 30;//30
	int UNLOAD_DISTANCE = 34;//34

	ChunkPool chunkPool;

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

	std::vector<uint8_t> REGION_BUFFER;

	uint64_t GetChunkKey(int32_t cx, int32_t cz) const {
		return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32 | static_cast<uint32_t>(cz));
	}


	void WakeNearbyWater(int bx, int by, int bz);

	void InitRegionSkyLight();
	void RebuildSkylightRegion(int32_t cx, int32_t cz);
	void RebuildChunkSkylightFast(int32_t cx, int32_t cz);

	void SeedChunkSkylightTop(int32_t cx, int32_t cz);

	uint8_t ComputeSkyAttenuation(unsigned int block);



	void MarkChunkMeshDirty(int32_t cx, int32_t cz);
	void MarkChunkLightDirty(int32_t cx, int32_t cz, bool urgent);
	void MarkChunkMeshDirtyByBlock(int bx, int by, int bz);
	

	void RebuildMeshQueue(int32_t curCx, int32_t curCz);
	void GatherUnloadCandidates(int32_t curCx, int32_t curCz);

	uint8_t GetSkylightGlobal(int bx, int by, int bz);
	bool SetSkylightGlobal(int bx, int by, int bz, uint8_t light);
	bool SetSkylightGlobalNoDirty(int bx, int by, int bz, uint8_t light);
	bool IsTransparentGlobal(int bx, int by, int bz);

	void PropagateSkylightAdd(int bx, int by, int bz);
	void PropagateSkylightRemove(int bx, int by, int bz, uint8_t oldLight);
	void PropagateSkylightAddNoDirty(int bx, int by, int bz, std::unordered_set<uint64_t>& dirtyChunks);

	uint8_t ComputeSkyLightFromNeighbors(int bx, int by, int bz);
	

	void ChunkGenerate(Chunk* c);

	void ProcessGenQueue();
	void ProcessUrgentLightQueue(int& lightBudged);
	void ProcessNormalLightQueue(int& lightBudged);
	void ProcessMeshQueue();
	void ProcessGpuDeletes();
	void ProcessUnloadQueue(int32_t curCx, int32_t curCz);
	void ProcessWaterQueue();

	bool ProcessOneGenJob();
	bool ProcessOneUrgentLightJob();
	bool ProcessOneNormalLightJob();
	bool ProcessOneMeshJob();
};

extern World* gWorld;