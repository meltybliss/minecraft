#include "World/world.h"
#include "Core/game.h"
#include <chrono>

World* gWorld = nullptr;

static double GetTimeMs() {
	using namespace std::chrono;
	return duration<double, std::milli>(
		steady_clock::now().time_since_epoch()
	).count();
}

static int32_t FloorDiv(int v, int b) {
	return static_cast<int32_t>(std::floor(static_cast<float>(v) / b));
}

World::World() : worldSeed(123456789u), meshQueue(ChunkPriority{ 0, 0 }), TNTRng(std::random_device{}()) {
	gWorld = this;

	for (int x = -RENDER_DISTANCE; x <= RENDER_DISTANCE; x++) {
		for (int z = -RENDER_DISTANCE; z <= RENDER_DISTANCE; z++) {
			spiralOffsets.push_back(Vec2{x, z });
		}
	}

	std::sort(spiralOffsets.begin(), spiralOffsets.end(), [](const Vec2& a, const Vec2& b) {

		int distA = a.x * a.x + a.z * a.z;
		int distB = b.x * b.x + b.z * b.z;

		return distA < distB;

	});

	for (size_t i = 0; i < spiralOffsets.size(); i++) {
		const Vec2& off = spiralOffsets[i];
		spiralRank[GetChunkKey(off.x, off.z)] = i;
	}

	InitRegionSkyLight();
}


void World::InitRegionSkyLight() {

	constexpr int REGION_W = Chunk::CHUNK_WIDTH * 3;
	REGION_BUFFER.resize(REGION_W * REGION_W * Chunk::CHUNK_HEIGHT, 0);
}

void World::Tick(float dt) {

	Vec3 pos = gGame->GetPlayer().getPos();
	float chunkWorldSize = static_cast<float>(Chunk::CHUNK_WIDTH * blockSize);

	int32_t curCx = static_cast<int32_t>(std::floor(pos.x / chunkWorldSize));
	int32_t curCz = static_cast<int32_t>(std::floor(pos.z / chunkWorldSize));

	bool movedToNewChunk = (curCx != lastPlrChunkCx) || (curCz != lastPlrChunkCz);

	
	for (const auto& off : spiralOffsets) {
		int32_t x = curCx + off.x;
		int32_t z = curCz + off.z;

			
		uint64_t key = GetChunkKey(x, z);
		if (Chunks.find(key) == Chunks.end()) {


			auto c = std::make_shared<Chunk>();
			c->cx = x;
			c->cz = z;
			c->chunkSeed = Chunk::makeChunkSeed(gWorld->getWorldSeed(), c->cx, c->cz);

			if (!c->isQueuedForGen) {
				generationQueue.push_back(c);
				c->isQueuedForGen = true;
				Chunks[key] = c;
					
			}

		}
		
	}

	


	if (movedToNewChunk) {
		lastPlrChunkCx = curCx;
		lastPlrChunkCz = curCz;

		RebuildMeshQueue(curCx, curCz);
		GatherUnloadCandidates(curCx, curCz);
	}

	for (auto& entity : entities) {

		entity->Tick(dt);

	}

	for (auto& p : pendingEntities) {
		entities.push_back(std::move(p));
	}

	pendingEntities.clear();

	std::erase_if(entities, [](const std::unique_ptr<Entity>& e) {
		return e->IsDead();
	});

	static int waterProcCount = 0;
	waterProcCount++;

	int lightCommonBudged = 3;
	if (!generationQueue.empty()) {
		ProcessGenQueue();
		ProcessUrgentLightQueue(lightCommonBudged);

	}
	else {
		
		ProcessUrgentLightQueue(lightCommonBudged);
		ProcessNormalLightQueue(lightCommonBudged);
	
		
	}

	ProcessMeshQueue();
	ProcessUnloadQueue(curCx, curCz);
	ProcessGpuDeletes();

	

	if (waterProcCount >= 35) {
		ProcessWaterQueue();
		waterProcCount = 0;
	}
}


void World::render(GLuint program) {
	
	glUseProgram(program);
	Mat4 identity = Mat4::Identity();
	glUniformMatrix4fv(glGetUniformLocation(program, "uModel"), 1, GL_FALSE, identity.m);

	glUniform1f(glGetUniformLocation(program, "uFlash"), 0.0f);
	for (auto& item : Chunks) {
		auto& c = item.second;
		if (!c) continue;

		c->renderSolid(program);
	}

	for (auto& entity : entities) {
		entity->Render(program);
	}

	glUniformMatrix4fv(glGetUniformLocation(program, "uModel"), 1, GL_FALSE, identity.m);
	glUniform1f(glGetUniformLocation(program, "uFlash"), 0.0f);

	for (auto& item : Chunks) {
		auto& c = item.second;
		if (!c) continue;

		c->renderWater(program);

	}
}


unsigned int World::GetBlockGlobal(int bx, int by, int bz) {
	int32_t cx = FloorDiv(bx, Chunk::CHUNK_WIDTH);
	int32_t cz = FloorDiv(bz, Chunk::CHUNK_WIDTH);

	uint64_t key = GetChunkKey(cx, cz);
	auto it = Chunks.find(key);
	if (it == Chunks.end()) return (unsigned int)BlockType::AIR;

	Chunk* c = it->second.get();

	int lx = bx - cx * Chunk::CHUNK_WIDTH;
	int ly = by;
	int lz = bz - cz * Chunk::CHUNK_WIDTH;

	return c->Get(lx, ly, lz);
}

Chunk* World::GetChunkPtr(int cx, int cz) {
	uint64_t key = GetChunkKey(cx, cz);
	if (Chunks.find(key) == Chunks.end()) return nullptr;
	return Chunks[key].get();
}


void World::MarkChunkMeshDirty(int32_t cx, int32_t cz) {
	uint64_t key = GetChunkKey(cx, cz);

	auto it = Chunks.find(key);
	if (it == Chunks.end() || !it->second) return;
	
	std::shared_ptr<Chunk> c = it->second;
	c->isMeshDirty = true;

	if (!c->isQueuedForMesh) {
	
		meshQueue.push(c);
		c->isQueuedForMesh = true;
	}
}

void World::MarkChunkLightDirty(int32_t cx, int32_t cz, bool urgent) {
	uint64_t key = GetChunkKey(cx, cz);

	auto it = Chunks.find(key);
	if (it == Chunks.end() || !it->second) return;

	std::shared_ptr<Chunk> c = it->second;
	c->isLightDirty = true;

	if (!c->isQueuedForLight) {
		if (urgent) {
			urgentLightQueue.push_back(key);
		}
		else {
			normalLightQueue.push_back(key);
		}
		
		c->isQueuedForLight = true;
	}


}


bool World::SetBlockGlobalForPlr(int bx, int by, int bz, unsigned int block) {//playerが直接呼ぶ用
	int32_t cx = FloorDiv(bx, Chunk::CHUNK_WIDTH);
	int32_t cz = FloorDiv(bz, Chunk::CHUNK_WIDTH);

	uint64_t key = GetChunkKey(cx, cz);
	auto it = Chunks.find(key);
	if (it == Chunks.end()) return false;

	Chunk* c = it->second.get();

	int lx = bx - cx * Chunk::CHUNK_WIDTH;
	int ly = by;
	int lz = bz - cz * Chunk::CHUNK_WIDTH;

	if (block != 0) {
		bool ok = c->isAirBlock(lx, ly, lz);

		if (ok && block == (unsigned int)BlockType::Water) {
			EnqueueWaterProc(bx, by, bz);
		}
		else if (!ok) {
			return false;
		}

	}
	else {
		WakeNearbyWater(bx, by, bz);
	}

	bool ok = c->Set(lx, ly, lz, block);
	if (!ok) return false;

	
	MarkChunkLightDirty(cx, cz, true);
	(cx, cz);
	c->isEdited = true;

	if (lx == 0) MarkChunkMeshDirty(cx - 1, cz);
	if (lx == Chunk::CHUNK_WIDTH - 1) MarkChunkMeshDirty(cx + 1, cz);
	if (lz == 0) MarkChunkMeshDirty(cx, cz - 1);
	if (lz == Chunk::CHUNK_WIDTH - 1) MarkChunkMeshDirty(cx, cz + 1);

	return true;
}

bool World::SetBlockGlobalForProgram(int bx, int by, int bz, unsigned int block) {//playerが設置するときに使わない
	int32_t cx = FloorDiv(bx, Chunk::CHUNK_WIDTH);
	int32_t cz = FloorDiv(bz, Chunk::CHUNK_WIDTH);

	uint64_t key = GetChunkKey(cx, cz);
	auto it = Chunks.find(key);
	if (it == Chunks.end()) return false;

	Chunk* c = it->second.get();

	int lx = bx - cx * Chunk::CHUNK_WIDTH;
	int ly = by;
	int lz = bz - cz * Chunk::CHUNK_WIDTH;


	WakeNearbyWater(bx, by, bz);

	bool ok = c->Set(lx, ly, lz, block);
	if (!ok) return false;
	

	MarkChunkLightDirty(cx, cz, true);
	c->isEdited = true;

	if (lx == 0) MarkChunkMeshDirty(cx - 1, cz);
	if (lx == Chunk::CHUNK_WIDTH - 1) MarkChunkMeshDirty(cx + 1, cz);
	if (lz == 0) MarkChunkMeshDirty(cx, cz - 1);
	if (lz == Chunk::CHUNK_WIDTH - 1) MarkChunkMeshDirty(cx, cz + 1);

	return true;
}



bool World::SetBlockByRay(Ray& ray, unsigned int block, float maxDist) {
	HitResult hit = TraceRay(ray, maxDist);
	if (!hit.isHit) return false;

	return this->SetBlockGlobalForPlr(hit.hitPos.x, hit.hitPos.y, hit.hitPos.z, block);
	
}


uint8_t World::GetSkylightGlobal(int bx, int by, int bz) {
	int32_t cx = FloorDiv(bx, Chunk::CHUNK_WIDTH);
	int32_t cz = FloorDiv(bz, Chunk::CHUNK_WIDTH);

	uint64_t key = GetChunkKey(cx, cz);

	auto it = Chunks.find(key);
	if (it == Chunks.end() || !it->second) return 0;

	Chunk* c = it->second.get();

	int lx = bx - cx * Chunk::CHUNK_WIDTH;
	int ly = by;
	int lz = bz - cx * Chunk::CHUNK_WIDTH;

	if (lx < 0 || lx >= Chunk::CHUNK_WIDTH) return 0;
	if (ly < 0 || ly >= Chunk::CHUNK_HEIGHT) return 0;
	if (lz < 0 || lz >= Chunk::CHUNK_WIDTH) return 0;

	int idx = Chunk::Index(lx, ly, lz);

	return c->blocks[idx].skyLight;
}


bool World::SetSkylightGlobal(int bx, int by, int bz, uint8_t light) {
	int32_t cx = FloorDiv(bx, Chunk::CHUNK_WIDTH);
	int32_t cz = FloorDiv(bz, Chunk::CHUNK_WIDTH);

	uint64_t key = GetChunkKey(cx, cz);

	auto it = Chunks.find(key);
	if (it == Chunks.end() || !it->second) return false;

	Chunk* c = it->second.get();

	int lx = bx - cx * Chunk::CHUNK_WIDTH;
	int ly = by;
	int lz = bz - cx * Chunk::CHUNK_WIDTH;

	if (lx < 0 || lx >= Chunk::CHUNK_WIDTH) return false;
	if (ly < 0 || ly >= Chunk::CHUNK_HEIGHT) return false;
	if (lz < 0 || lz >= Chunk::CHUNK_WIDTH) return false;

	int idx = Chunk::Index(lx, ly, lz);

	c->blocks[idx].skyLight = light;
	MarkChunkMeshDirtyByBlock(bx, by, bz);

	return true;
}


bool World::SetSkylightGlobalNoDirty(int bx, int by, int bz, uint8_t light) {
	int32_t cx = FloorDiv(bx, Chunk::CHUNK_WIDTH);
	int32_t cz = FloorDiv(bz, Chunk::CHUNK_WIDTH);

	uint64_t key = GetChunkKey(cx, cz);

	auto it = Chunks.find(key);
	if (it == Chunks.end() || !it->second) return false;

	Chunk* c = it->second.get();

	int lx = bx - cx * Chunk::CHUNK_WIDTH;
	int ly = by;
	int lz = bz - cx * Chunk::CHUNK_WIDTH;

	if (lx < 0 || lx >= Chunk::CHUNK_WIDTH) return false;
	if (ly < 0 || ly >= Chunk::CHUNK_HEIGHT) return false;
	if (lz < 0 || lz >= Chunk::CHUNK_WIDTH) return false;

	int idx = Chunk::Index(lx, ly, lz);

	c->blocks[idx].skyLight = light;
	

	return true;
}



bool World::IsTransparentGlobal(int bx, int by, int bz) {
	unsigned int b = GetBlockGlobal(bx, by, bz);
	return b == 0 ||
		b == (unsigned int)BlockType::Water ||
		b == (unsigned int)BlockType::Leave;

}


void World::MarkChunkMeshDirtyByBlock(int bx, int by, int bz) {
	int32_t cx = FloorDiv(bx, Chunk::CHUNK_WIDTH);
	int32_t cz = FloorDiv(bz, Chunk::CHUNK_WIDTH);

	MarkChunkMeshDirty(cx, cz);

	int lx = bx - cx * Chunk::CHUNK_WIDTH;
	int lz = bz - cz * Chunk::CHUNK_WIDTH;

	if (lx == 0) MarkChunkMeshDirty(cx - 1, cz);
	else if (lx == Chunk::CHUNK_WIDTH - 1) MarkChunkMeshDirty(cx + 1, cz);

	if (lz == 0) MarkChunkMeshDirty(cx, cz - 1);
	else if (lz == Chunk::CHUNK_WIDTH - 1) MarkChunkMeshDirty(cx, cz + 1);
}


uint8_t World::ComputeSkyLightFromNeighbors(int bx, int by, int bz) {
	if (!IsTransparentGlobal(bx, by, bz)) return 0;

	uint8_t best = 0;

	uint8_t up = GetSkylightGlobal(bx, by + 1, bz);
	if (up == 15 && IsTransparentGlobal(bx, by + 1, bz)) {
		best = 15;
	}
	else if (up > 1) {
		best = std::max(best, static_cast<uint8_t>(up - 1));
	}


	//right, left, front, back, down
	const int dirs[5][3] = {
		{1, 0, 0},
		{-1, 0, 0},
		{0, 0, 1},
		{0, 0, -1},
		{0, -1, 0},

	};

	for (auto& d : dirs) {
		int nx = bx + d[0];
		int ny = by + d[1];
		int nz = bz + d[2];

		uint8_t newLight = GetSkylightGlobal(nx, ny, nz);
		if (newLight <= 1) continue;

		best = std::max(best, static_cast<uint8_t>(newLight - 1));
	}

	return best;
}


void World::PropagateSkylightAdd(int bx, int by, int bz) {

	uint8_t startLight = ComputeSkyLightFromNeighbors(bx, by, bz);
	uint8_t curLight = GetSkylightGlobal(bx, by, bz);
	
	if (startLight <= curLight) return;

	SetSkylightGlobal(bx, by, bz, startLight);

	std::queue<LightNode> q;
	q.push({ bx, by, bz, startLight });

	//right, left, front, back, down
	const int dirs[5][3] = {
		{1, 0, 0},
		{-1, 0, 0},
		{0, 0, 1},
		{0, 0, -1},
		{0, -1, 0},

	};

	while (!q.empty()) {
		LightNode cur = q.front();
		q.pop();

		curLight = cur.skyLight;

		for (auto& d : dirs) {
			

			int nx = cur.x + d[0];
			int ny = cur.y + d[1];
			int nz = cur.z + d[2];

			if (!IsTransparentGlobal(nx, ny, nz)) continue;

			uint8_t oldLight = GetSkylightGlobal(nx, ny, nz);
			uint8_t newLight = oldLight;
			if (d[0] == 0 && d[1] == -1 && d[2] == 0 && curLight == 15) {
				newLight = 15;
			}
			else {
				if (curLight <= 1) continue;

				unsigned int b = GetBlockGlobal(nx, ny, nz);
				if (b == 0) {
					newLight = curLight - 1;
				}
				else if (b == (unsigned int)BlockType::Leave) {
					newLight = curLight - 3;
				}
				else if (b == (unsigned int)BlockType::Water) {
					newLight = curLight - 2;
				}
			}

			if (newLight > oldLight) {
				q.push({ nx, ny, nz, newLight });
				SetSkylightGlobal(nx, ny, nz, newLight);
			}
		}
	}


}


void World::PropagateSkylightRemove(int bx, int by, int bz, uint8_t oldLight) {
	std::queue<RemoveNode> removeQ;
	std::queue<LightNode> addQ;

	SetSkylightGlobal(bx, by, bz, 0);
	removeQ.push({ bx, by, bz, oldLight });

	const int dirs[6][3] = {
		{ 1, 0, 0 }, {-1, 0, 0 },
		{ 0, 0, 1 }, { 0, 0,-1 },
		{ 0, 1, 0 }, { 0,-1, 0 }
	};

	while (!removeQ.empty()) {
		RemoveNode cur = removeQ.front();
		removeQ.pop();

		for (const auto& d : dirs) {
			int nx = cur.x + d[0];
			int ny = cur.y + d[1];
			int nz = cur.z + d[2];

			if (ny < 0 || ny >= Chunk::CHUNK_HEIGHT) continue;
			if (!IsTransparentGlobal(nx, ny, nz)) continue;

			uint8_t neighborLight = GetSkylightGlobal(nx, ny, nz);
			if (neighborLight == 0) continue;

			// 今の削除元の光に依存してそうなら消す
			if (neighborLight < cur.oldLight) {
				SetSkylightGlobal(nx, ny, nz, 0);
				removeQ.push({ nx, ny, nz, neighborLight });
			}
			else {
				// 別経路の光が残ってる可能性があるので add 側へ
				addQ.push({ nx, ny, nz, neighborLight });
			}
		}
	}

	while (!addQ.empty()) {
		LightNode cur = addQ.front();
		addQ.pop();

		PropagateSkylightAdd(cur.x, cur.y, cur.z);
	}
}


HitResult World::TraceRay(Ray& ray, float maxDist) {
	HitResult hit = { false, {0,0,0}, {0,0,0}, 0.0f };

	Vec3 origin = {
		ray.origin.x / blockSize,
		ray.origin.y / blockSize,
		ray.origin.z / blockSize
	};

	int x = (int)std::floor(origin.x);
	int y = (int)std::floor(origin.y);
	int z = (int)std::floor(origin.z);

	Vec3 deltaDist = {
		(ray.dir.x == 0.0f) ? 1e30f : std::abs(1.0f / ray.dir.x),
		(ray.dir.y == 0.0f) ? 1e30f : std::abs(1.0f / ray.dir.y),
		(ray.dir.z == 0.0f) ? 1e30f : std::abs(1.0f / ray.dir.z)
	};

	Vec3 sideDist;
	int stepX, stepY, stepZ;

	if (ray.dir.x < 0) {
		stepX = -1;
		sideDist.x = (origin.x - x) * deltaDist.x;
	}
	else {
		stepX = 1;
		sideDist.x = (x + 1.0f - origin.x) * deltaDist.x;
	}

	if (ray.dir.y < 0) {
		stepY = -1;
		sideDist.y = (origin.y - y) * deltaDist.y;
	}
	else {
		stepY = 1;
		sideDist.y = (y + 1.0f - origin.y) * deltaDist.y;
	}

	if (ray.dir.z < 0) {
		stepZ = -1;
		sideDist.z = (origin.z - z) * deltaDist.z;
	}
	else {
		stepZ = 1;
		sideDist.z = (z + 1.0f - origin.z) * deltaDist.z;
	}

	float t = 0.0f;
	while (t < maxDist / blockSize) {
		if (sideDist.x < sideDist.y && sideDist.x < sideDist.z) {
			t = sideDist.x;
			sideDist.x += deltaDist.x;
			x += stepX;
			hit.normal = { -(float)stepX, 0, 0 };
		}
		else if (sideDist.y < sideDist.z) {
			t = sideDist.y;
			sideDist.y += deltaDist.y;
			y += stepY;
			hit.normal = { 0, -(float)stepY, 0 };
		}
		else {
			t = sideDist.z;
			sideDist.z += deltaDist.z;
			z += stepZ;
			hit.normal = { 0, 0, -(float)stepZ };
		}

		if (GetBlockGlobal(x, y, z) != (unsigned int)BlockType::AIR) {
			hit.isHit = true;
			hit.hitPos = { (float)x, (float)y, (float)z }; // block座標で返す
			hit.dist = t * blockSize;                      // world距離に戻す
			return hit;
		}
	}

	return hit;
}


#pragma region WORLD_LIGHT_CALC_BUFFER
void World::RebuildSkylightRegion(int32_t cx, int32_t cz) {

	int startBx = (cx - 1) * Chunk::CHUNK_WIDTH;
	int endBx = (cx + 2) * Chunk::CHUNK_WIDTH; // 1つ先の終端
	int startBz = (cz - 1) * Chunk::CHUNK_WIDTH;
	int endBz = (cz + 2) * Chunk::CHUNK_WIDTH;
	
	for (int bx = startBx; bx < endBx; bx++) {
		for (int bz = startBz; bz < endBz; bz++) {
			for (int y = 0; y < Chunk::CHUNK_HEIGHT; y++) {
				SetSkylightGlobalNoDirty(bx, y, bz, 0);
			}
		}
	}

	std::queue<LightNode> q;

	for (int bx = startBx; bx < endBx; bx++) {
		for (int bz = startBz; bz < endBz; bz++) {
			for (int y = Chunk::CHUNK_HEIGHT - 1; y >= 0; y--) {
				unsigned int block = GetBlockGlobal(bx, y, bz);

				if (block == 0) {
					SetSkylightGlobalNoDirty(bx, y, bz, 15);
					q.push({ bx, y, bz, 15 });
				}
				else {
					break;
				}
			}
		}
	}

	while (!q.empty()) {
		LightNode cur = q.front();

		q.pop();

		uint8_t curLight = cur.skyLight;

		if (curLight == 0) continue;

		//right, left, front, back, down
		const int dirs[5][3] = {
			{1, 0, 0},
			{-1, 0, 0},
			{0, 0, 1},
			{0, 0, -1},
			{0, -1, 0},

		};

		for (const auto& dir : dirs) {
			int nx = cur.x + dir[0];
			int ny = cur.y + dir[1];
			int nz = cur.z + dir[2];

			if (ny < 0 || ny >= Chunk::CHUNK_HEIGHT) continue;

			unsigned int nBlock = GetBlockGlobal(nx, ny, nz);

			if (!IsTransparentGlobal(nx, ny, nz)) continue;

			uint8_t newLight = 0;
			uint8_t oldLight = GetSkylightGlobal(nx, ny, nz);

			if (dir[0] == 0 && dir[1] == -1 && dir[2] == 0 && curLight == 15) {

				newLight = curLight;
			}
			else {
				if (curLight <= 1) continue;

				if (nBlock == 0) {

					newLight = curLight - 1;
				}
				else if (nBlock == (unsigned int)BlockType::Leave) {
					newLight = curLight - 3;
				}
				else if (nBlock == (unsigned int)BlockType::Water) {
					newLight = curLight - 2;
				}
			}

			if (newLight > oldLight) {
				SetSkylightGlobalNoDirty(nx, ny, nz, newLight);
				q.push({ nx, ny, nz, newLight });
			}

		}


	}

	for (int dx = -1; dx <= 1; dx++) {
		for (int dz = -1; dz <= 1; dz++) {
			MarkChunkMeshDirty(cx + dx, cz + dz);
		}
	}
}
#pragma endregion WORLD_LIGHT_CALC_BUFFER


void World::RebuildMeshQueue(int32_t curCx, int32_t curCz) {
	std::vector<std::shared_ptr<Chunk>> pedding;

	while (!meshQueue.empty()) {
		auto c = meshQueue.top();
		meshQueue.pop();

		if (!c) continue;

		c->isQueuedForMesh = false;

		if (!c->isMeshDirty) continue;

		pedding.push_back(c);

	}

	std::priority_queue<
		std::shared_ptr<Chunk>,
		std::vector<std::shared_ptr<Chunk>>,
		ChunkPriority> newQueue(ChunkPriority{curCx, curCz});

	for (const auto& c : pedding) {
		newQueue.push(c);
		c->isQueuedForMesh = true;
	}

	meshQueue = std::move(newQueue);

 }


void World::GatherUnloadCandidates(int32_t curCx, int32_t curCz) {

	for (const auto& [key, c] : Chunks) {
		if (c == nullptr) continue;

		int32_t dx = std::abs(c->cx - curCx);
		int32_t dz = std::abs(c->cz - curCz);

		if (dx >= UNLOAD_DISTANCE || dz >= UNLOAD_DISTANCE) {
			if (c->isQueuedForUnload) continue;

			unloadQueue.push_back(key);
			c->isQueuedForUnload = true;
		}
	}

}

void World::ChunkGenerate(Chunk* c) {

	terrainGen.Generate(c);
	caveGen.ApplyCaves(c);
	MarkChunkLightDirty(c->cx, c->cz, false);
}


void World::Ignite(int bx, int by, int bz, float timer,
					bool hasExplosionSource,
					int ex, int ey, int ez) {

	if (GetBlockGlobal(bx, by, bz) != (unsigned int)BlockType::TNT) return;
	
	this->SetBlockGlobalForPlr(bx, by, bz, 0);

	auto TNT = std::make_unique<TNTEntity>(Vec3{ (float)bx, (float)by, (float)bz }, 
		timer);

	if (hasExplosionSource) {

		Vec3 dir = Normalize(Vec3{
			(float)bx - ex,
			(float)by - ey,
			(float)bz - ez
		});

		Vec3 vel = dir * 3.0f;
		vel.y += 6.0f;
		TNT->setVel(vel);
	}
	else {
		std::uniform_real_distribution<float> dirDist(-0.02f, 0.02f);

		TNT->setVel(Vec3{ dirDist(TNTRng), 6.f, dirDist(TNTRng) });
	}
	

	pendingEntities.push_back(std::move(TNT));
}


void World::WakeNearbyWater(int bx, int by, int bz) {
	static const std::array<BlockPos, 6> dirs{ {
		{0, 1, 0}, {0, -1, 0},
		{1, 0, 0}, {-1, 0, 0},
		{0, 0, 1}, {0, 0, -1}
	}};

	for (auto& dir : dirs) {
		if (GetBlockGlobal(bx + dir.x, by + dir.y, bz + dir.z) == (unsigned int)BlockType::Water) {
			EnqueueWaterProc(bx + dir.x, by + dir.y, bz + dir.z);
		}
	}
}

#pragma region QueueProcesses
void World::ProcessGpuDeletes() {
	int deleteBudged = 3;

	while (deleteBudged-- > 0 && !gpuDeleteQueue.empty()) {
		auto job = gpuDeleteQueue.front();
		gpuDeleteQueue.pop_front();

		if (job.vao != 0) glDeleteVertexArrays(1, &job.vao);
		if (job.vbo != 0) glDeleteBuffers(1, &job.vbo);

	}


}


void World::ProcessGenQueue() {
	int genBudget = 3;
	while (genBudget-- > 0 && !generationQueue.empty()) {
		std::weak_ptr<Chunk> wp = generationQueue.front();
		generationQueue.pop_front();


		if (auto c = wp.lock()) {
			
			ChunkGenerate(c.get());

			c->isGenerated = true;
			c->isQueuedForGen = false;
			
			
		}
	}


}


void World::ProcessUrgentLightQueue(int& lightBudged) {
	while (lightBudged > 0 && !urgentLightQueue.empty()) {


		uint64_t key = urgentLightQueue.front();
		urgentLightQueue.pop_front();

		auto it = Chunks.find(key);
		if (it == Chunks.end() || !it->second) continue;

		std::shared_ptr<Chunk> c = it->second;

		if (!c) continue;

		c->isQueuedForLight = false;
		if (!c->isLightDirty) continue;

		this->RebuildSkylightRegion(c->cx, c->cz);


		lightBudged--;

	}

}


void World::ProcessNormalLightQueue(int& lightBudged) {
	while (lightBudged > 0 && !normalLightQueue.empty()) {

		
		uint64_t key = normalLightQueue.front();
		normalLightQueue.pop_front();

		auto it = Chunks.find(key);
		if (it == Chunks.end() || !it->second) continue;

		std::shared_ptr<Chunk> c = it->second;

		if (!c) continue;

		c->isQueuedForLight = false;
		if (!c->isLightDirty) continue;

		this->RebuildSkylightRegion(c->cx, c->cz);

		lightBudged--;


	}

}


void World::ProcessMeshQueue() {

	int meshBudget = generationQueue.empty() ? 3 : 1;

	std::vector<std::shared_ptr<Chunk>> deferred;

	while (meshBudget > 0 && !meshQueue.empty()) {
		std::shared_ptr<Chunk> c = meshQueue.top();
		meshQueue.pop();

		if (!c) continue;

		c->isQueuedForMesh = false;
		if (!c->isMeshDirty) continue;
		if (c->isLightDirty) {
			deferred.push_back(c);
			continue;
		}

		meshBuilder.BuildMesh(c.get());

		meshBudget--;
	}

	for (auto& c : deferred) {
		if (!c) continue;
		if (!c->isQueuedForMesh) {
			meshQueue.push(c);
			c->isQueuedForMesh = true;
		}
	}

}


void World::ProcessUnloadQueue(int32_t curCx, int32_t curCz) {


	int unloadBudget = 5;
	while (unloadBudget-- > 0 && !unloadQueue.empty()) {
		uint64_t key = unloadQueue.front();
		unloadQueue.pop_front();

		auto it = Chunks.find(key);
		if (it == Chunks.end()) continue;

		auto c = it->second;
		if (!c) {
			Chunks.erase(key);
			continue;
		}

		int32_t dx = std::abs(c->cx - curCx);
		int32_t dz = std::abs(c->cz - curCz);

		if (dx >= UNLOAD_DISTANCE || dz >= UNLOAD_DISTANCE) {
			c->isQueuedForGen = false;
			c->isQueuedForMesh = false;
			c->isQueuedForUnload = false;
			Chunks.erase(key);
			
		}
		else {
			c->isQueuedForUnload = false;
		}


	}
}


void World::ProcessWaterQueue() {

	int waterBudged = 500;

	static const std::array<BlockPos, 4> sideDirs{{
		{1, 0, 0},
		{-1, 0, 0},
		{0, 0, -1},
		{0, 0, 1}
	}};

	static const BlockPos down = { 0, -1, 0 };

	int count = std::min((int)waterProcQueue.size(), waterBudged);

	while (count-- > 0 && !waterProcQueue.empty()) {

		auto data = waterProcQueue.front();

		auto pos = data.pos;

		waterProcQueue.pop_front();

		if (*data.level <= 0) continue;

		(*data.level)--;

		unsigned int b = GetBlockGlobal(pos.x, pos.y, pos.z);

		if (b != (unsigned int)BlockType::Water) {
			SetBlockGlobalForProgram(pos.x, pos.y, pos.z, (unsigned int)BlockType::Water);
		}

		BlockPos next;
		if (GetBlockGlobal(pos.x, pos.y - 1, pos.z) == 0) {
			next = pos + down;
			data.pos = next;
			SetBlockGlobalForProgram(next.x, next.y, next.z, (unsigned int)BlockType::Water);
			waterProcQueue.push_back(data);
		}
		else {
			auto dirs = sideDirs;
			std::shuffle(dirs.begin(), dirs.end(), TNTRng);

			for (const auto& d : dirs) {
				next = pos + d;
				if (GetBlockGlobal(next.x, next.y, next.z) == 0) {
					data.pos = next;
					SetBlockGlobalForProgram(next.x, next.y, next.z, (unsigned int)BlockType::Water);
					waterProcQueue.push_back(data);

				}
			}
		}

		
	}
}

bool World::ProcessOneGenJob() {
	if (generationQueue.empty()) return false;

	std::weak_ptr<Chunk> wp = generationQueue.front();
	generationQueue.pop_front();

	if (auto c = wp.lock()) {
		ChunkGenerate(c.get());

		c->isGenerated = true;
		c->isQueuedForGen = false;
		return true;
	}

	return false;
}

bool World::ProcessOneUrgentLightJob() {
	if (urgentLightQueue.empty()) return false;

	uint64_t key = urgentLightQueue.front();
	urgentLightQueue.pop_front();

	auto it = Chunks.find(key);
	if (it == Chunks.end() || !it->second) return false;

	std::shared_ptr<Chunk> c = it->second;
	if (!c) return false;

	c->isQueuedForLight = false;
	if (!c->isLightDirty) return false;

	RebuildSkylightRegion(c->cx, c->cz);
	return true;
}

bool World::ProcessOneNormalLightJob() {
	if (normalLightQueue.empty()) return false;

	uint64_t key = normalLightQueue.front();
	normalLightQueue.pop_front();

	auto it = Chunks.find(key);
	if (it == Chunks.end() || !it->second) return false;

	std::shared_ptr<Chunk> c = it->second;
	if (!c) return false;

	c->isQueuedForLight = false;
	if (!c->isLightDirty) return false;

	RebuildSkylightRegion(c->cx, c->cz);
	return true;
}


bool World::ProcessOneMeshJob() {
	if (meshQueue.empty()) return false;

	std::vector<std::shared_ptr<Chunk>> deferred;
	bool built = false;

	int tries = static_cast<int>(meshQueue.size());

	while (tries-- > 0 && !meshQueue.empty()) {
		std::shared_ptr<Chunk> c = meshQueue.top();
		meshQueue.pop();

		if (!c) continue;

		c->isQueuedForMesh = false;
		if (!c->isMeshDirty) continue;

		if (c->isLightDirty) {
			deferred.push_back(c);
			continue;
		}

		meshBuilder.BuildMesh(c.get());
		built = true;
		break;
	}

	for (auto& c : deferred) {
		if (!c) continue;
		if (!c->isQueuedForMesh) {

			meshQueue.push(c);
			c->isQueuedForMesh = true;
		}
	}

	
	return built;
}

#pragma endregion QueueProcesses