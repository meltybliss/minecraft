#include "world.h"
#include "game.h"

World* gWorld = nullptr;


static int32_t FloorDiv(int v, int b) {
	return static_cast<int32_t>(std::floor(static_cast<float>(v) / b));
}

World::World() : worldSeed(123456789u) {
	gWorld = this;
}

void World::Tick() {

	Vec3 pos = gGame->GetPlayer().getPos();
	float chunkWorldSize = static_cast<float>(Chunk::CHUNK_WIDTH * blockSize);

	int32_t curCx = static_cast<int32_t>(std::floor(pos.x / chunkWorldSize));
	int32_t curCz = static_cast<int32_t>(std::floor(pos.z / chunkWorldSize));


	for (int32_t x = curCx - RENDER_DISTANCE; x <= curCx + RENDER_DISTANCE; x++) {
		for (int32_t z = curCz - RENDER_DISTANCE; z <= curCz + RENDER_DISTANCE; z++) {

			
			uint64_t key = GetChunkKey(x, z);
			if (Chunks.find(key) == Chunks.end()) {


				auto c = std::make_unique<Chunk>();
				c->cx = x;
				c->cz = z;

				if (!c->isQueuedForGen) {
					generationQueue.push_back(c.get());
					c->isQueuedForGen = true;
					Chunks[key] = std::move(c);
					
				}

				
			}
		}
	}

	std::erase_if(Chunks, [&](const auto& item) {
		const auto& c = item.second;
		if (c == nullptr) return false;

		int32_t dx = std::abs(c->cx - curCx);
		int32_t dz = std::abs(c->cz - curCz);

		if (dx >= UNLOAD_DISTANCE || dz >= UNLOAD_DISTANCE) {
			c->isQueuedForGen = false;
			c->isQueuedForMesh = false;
			return true;
		}

		return false;
	});


	int genBudget = 3;
	while (genBudget-- > 0 && !generationQueue.empty()) {
		Chunk* c = generationQueue.front();
		generationQueue.pop_front();

		uint64_t key = GetChunkKey(c->cx, c->cz);
		if (Chunks.find(key) == Chunks.end() || Chunks[key].get() != c) {
			continue;
		}
		
		c->generate();
		c->isQueuedForGen = false;

		MarkChunkDirty(c->cx, c->cz);
		MarkChunkDirty(c->cx + 1, c->cz);
		MarkChunkDirty(c->cx - 1, c->cz);
		MarkChunkDirty(c->cx, c->cz + 1);
		MarkChunkDirty(c->cx, c->cz - 1);
	}


	int meshBudget = 3;
	while (meshBudget-- > 0 && !meshQueue.empty()) {
		Chunk* c = meshQueue.front();
		meshQueue.pop_front();

		uint64_t key = GetChunkKey(c->cx, c->cz);
		if (Chunks.find(key) == Chunks.end() || Chunks[key].get() != c) {
			continue;
		}

		c->buildMesh();
		c->isQueuedForMesh = false;

	}

}


void World::render() {
	for (auto& item : Chunks) {
		auto& c = item.second;
		
		c->render();
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


void World::MarkChunkDirty(int32_t cx, int32_t cz) {
	uint64_t key = GetChunkKey(cx, cz);

	auto it = Chunks.find(key);
	if (it == Chunks.end() || !it->second) return;
	
	Chunk* c = it->second.get();
	c->isDirty = true;

	if (!c->isQueuedForMesh) {
		meshQueue.push_back(c);
		c->isQueuedForMesh = true;
	}
}

bool World::SetBlockGlobal(int bx, int by, int bz, unsigned int block) {
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
		if (!ok) return false;
	}

	bool ok = c->Set(lx, ly, lz, block);
	if (!ok) return false;

	MarkChunkDirty(cx, cz);

	if (lx == 0) MarkChunkDirty(cx - 1, cz);
	if (lx == Chunk::CHUNK_WIDTH - 1) MarkChunkDirty(cx + 1, cz);
	if (lz == 0) MarkChunkDirty(cx, cz - 1);
	if (lz == Chunk::CHUNK_WIDTH - 1) MarkChunkDirty(cx, cz + 1);

	return true;
}


bool World::SetBlockByRay(Ray& ray, unsigned int block, float maxDist) {
	HitResult hit = TraceRay(ray, maxDist);
	if (!hit.isHit) return false;

	return this->SetBlockGlobal(hit.hitPos.x, hit.hitPos.y, hit.hitPos.z, block);
	
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
			hit.hitPos = { (float)x, (float)y, (float)z }; // blockÀ•W‚Å•Ô‚·
			hit.dist = t * blockSize;                      // world‹——£‚É–ß‚·
			return hit;
		}
	}

	return hit;
}

