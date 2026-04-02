#include "world.h"
#include "game.h"

World* gWorld = nullptr;


static int32_t FloorDiv(int v, int b) {
	return static_cast<int32_t>(std::floor(static_cast<float>(v) / b));
}

World::World() {
	gWorld = this;
}

void World::Tick() {

	Vec3 pos = gGame->cam.pos;
	float chunkWorldSize = static_cast<float>(Chunk::CHUNK_WIDTH * blockSize);

	int32_t curCx = static_cast<int32_t>(std::floor(pos.x / chunkWorldSize));
	int32_t curCz = static_cast<int32_t>(std::floor(pos.z / chunkWorldSize));


	for (int32_t x = curCx - RENDER_DISTANCE; x <= curCx + RENDER_DISTANCE; x++) {
		for (int32_t z = curCz - RENDER_DISTANCE; z <= curCz + RENDER_DISTANCE; z++) {

			if (std::abs(x) >= MaxCX || std::abs(z) >= MaxCZ) continue;

			uint64_t key = GetChunkKey(x, z);
			if (Chunks.find(key) == Chunks.end()) {


				auto c = std::make_unique<Chunk>();
				c->cx = x;
				c->cz = z;

				c->generate();
				Chunks[key] = std::move(c);

				MarkChunkDirty(x, z);
				MarkChunkDirty(x + 1, z);
				MarkChunkDirty(x - 1, z);
				MarkChunkDirty(x, z + 1);
				MarkChunkDirty(x, z - 1);
				
			}
		}
	}

	std::erase_if(Chunks, [&](const auto& item) {
		const auto& c = item.second;
		if (c == nullptr) return false;

		int32_t dx = std::abs(c->cx - curCx);
		int32_t dz = std::abs(c->cz - curCz);

		return (dx >= UNLOAD_DISTANCE || dz >= UNLOAD_DISTANCE);

	});
}


void World::render() {
	for (auto& item : Chunks) {
		auto& c = item.second;
		
		if (c->isDirty) {

			c->buildMesh();
		}

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
	
	it->second->isDirty = true;
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

