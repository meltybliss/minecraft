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


unsigned int World::GetBlockGlobal(int wx, int wy, int wz) {
	int32_t cx = FloorDiv(wx, Chunk::CHUNK_WIDTH * blockSize);
	int32_t cz = FloorDiv(wz, Chunk::CHUNK_WIDTH * blockSize);

	uint64_t key = GetChunkKey(cx, cz);
	if (Chunks.find(key) == Chunks.end()) return 0;

	Chunk* c = Chunks[key].get();
	int lx = wx - cx * Chunk::CHUNK_WIDTH;
	int ly = wy;
	int lz = wz - cz * Chunk::CHUNK_WIDTH;

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

bool World::SetBlockGlobal(int wx, int wy, int wz, unsigned int block) {
	int32_t cx = FloorDiv(wx, Chunk::CHUNK_WIDTH * blockSize);
	int32_t cz = FloorDiv(wz, Chunk::CHUNK_WIDTH * blockSize);

	uint64_t key = GetChunkKey(cx, cz);
	if (Chunks.find(key) == Chunks.end()) return false;

	Chunk* c = Chunks[key].get();
	int lx = wx - cx * Chunk::CHUNK_WIDTH;
	int ly = wy;
	int lz = wz - cz * Chunk::CHUNK_WIDTH;

	return c->Set(lx, ly, lz, block);
}


bool World::SetBlockByRay(Ray& ray, float maxDist) {

}