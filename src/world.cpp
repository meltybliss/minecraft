#include "world.h"
#include "game.h"
World::World() {
	gWorld = this;
}

void World::Tick() {

	Vec3 pos = gGame->cam.pos;
	int32_t curCx = static_cast<int32_t>(std::floor(pos.x / (Chunk::CHUNK_WIDTH)));
	int32_t curCz = static_cast<int32_t>(std::floor(pos.z / (Chunk::CHUNK_WIDTH)));


	for (int32_t x = curCx - RENDER_DISTANCE; x <= curCx + RENDER_DISTANCE; x++) {
		for (int32_t z = curCz - RENDER_DISTANCE; z <= curCz + RENDER_DISTANCE; z++) {
			uint64_t key = GetChunkKey(x, z);
			if (Chunks.find(key) == Chunks.end()) {

				auto c = std::make_unique<Chunk>();
				c->cx = x;
				c->cz = z;

				c->generate();
				Chunks[key] = std::move(c);
				
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