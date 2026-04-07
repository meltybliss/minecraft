#include "World/TerrainGenerator.h"

namespace {
	constexpr uint32_t kTerrainSeedSalt = 1000u;
	constexpr uint32_t kTreeSeedSalt = 2000u;
}

void TerrainGenerator::Generate(Chunk* c) {
	FillTerrain(c);
	GenerateSea(c);

	std::mt19937 treeRng(c->chunkSeed + kTreeSeedSalt);
	GenerateTrees(c, treeRng);
}

void TerrainGenerator::FillTerrain(Chunk* c) {
	for (int z = 0; z < Chunk::CHUNK_WIDTH; z++) {
		for (int x = 0; x < Chunk::CHUNK_WIDTH; x++) {
			int wx = c->cx * Chunk::CHUNK_WIDTH + x;
			int wz = c->cz * Chunk::CHUNK_WIDTH + z;


			int surfaceY = GetSurfaceHeight(wx, wz);
			bool beach = (surfaceY <= kSeaLevel + 2) || IsNearSea(wx, wz);

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; y++) {
				BlockType b = BlockType::AIR;

				if (y == surfaceY) {
					b = beach ? BlockType::Sand : BlockType::Grass;
				}
				else if (y < surfaceY && y >= surfaceY - 4) {
					b = beach ? BlockType::Sand : BlockType::Dirt;
				}
				else if (y < surfaceY - 4) {
					b = BlockType::Stone;
				}


				c->blocks[c->Index(x, y, z)] = static_cast<unsigned int>(b);
			}
		}
	}
}

void TerrainGenerator::GenerateSea(Chunk* c) {
	for (int z = 0; z < Chunk::CHUNK_WIDTH; z++) {
		for (int x = 0; x < Chunk::CHUNK_WIDTH; x++) {
			int wx = c->cx * Chunk::CHUNK_WIDTH + x;
			int wz = c->cz * Chunk::CHUNK_WIDTH + z;

			int surfaceY = GetSurfaceHeight(wx, wz);

			if (surfaceY >= kSeaLevel) continue;
			if (surfaceY < 0 || surfaceY >= Chunk::CHUNK_HEIGHT) continue;

			// äCíÍÇçªÇ…Ç∑ÇÈ
			c->Set(x, surfaceY, z, (unsigned int)BlockType::Sand);

			// ï\ñ ïtãﬂÇ‡è≠ÇµçªÇ…Ç∑ÇÈ
			for (int y = surfaceY - 3; y < surfaceY; y++) {
				if (y < 0) continue;
				c->Set(x, y, z, (unsigned int)BlockType::Sand);
			}

			// seaLevelÇ‹Ç≈êÖÇ≈ñÑÇﬂÇÈ
			for (int y = surfaceY + 1; y <= kSeaLevel && y < Chunk::CHUNK_HEIGHT; y++) {
				c->Set(x, y, z, (unsigned int)BlockType::Water);
			}
		}
	}
}

void TerrainGenerator::GenerateTrees(Chunk* c, std::mt19937& rng) {
	std::uniform_int_distribution<int> chance(0, 99);
	std::uniform_int_distribution<int> heightDist(4, 6);


	for (int z = 2; z < Chunk::CHUNK_WIDTH - 2; z++) {
		for (int x = 2; x < Chunk::CHUNK_WIDTH - 2; x++) {
			if (chance(rng) >= 1) continue;

			int wx = c->cx * Chunk::CHUNK_WIDTH + x;
			int wz = c->cz * Chunk::CHUNK_WIDTH + z;
			int y = GetSurfaceHeight(wx, wz);

			if (y <= kSeaLevel + 1) continue;
			if (c->Get(x, y, z) != (unsigned int)BlockType::Grass) continue;
			if (y + 6 >= Chunk::CHUNK_HEIGHT) continue;

			int trunkH = heightDist(rng);

			for (int i = 1; i <= trunkH; i++) {
				c->Set(x, y + i, z, (unsigned int)BlockType::Wood);
			}

			int topY = y + trunkH;

			for (int dz = -2; dz <= 2; dz++) {
				for (int dx = -2; dx <= 2; dx++) {
					for (int dy = -1; dy <= 1; dy++) {
						if (std::abs(dx) == 2 && std::abs(dz) == 2 && dy == -1) continue;

						int lx = x + dx;
						int ly = topY + dy;
						int lz = z + dz;

						if (lx < 0 || lx >= Chunk::CHUNK_WIDTH ||
							lz < 0 || lz >= Chunk::CHUNK_WIDTH ||
							ly < 0 || ly >= Chunk::CHUNK_HEIGHT) continue;

						if (c->Get(lx, ly, lz) == (unsigned int)BlockType::AIR) {
							c->Set(lx, ly, lz, (unsigned int)BlockType::Leave);
						}
					}
				}
			}
		}
	}
}


bool TerrainGenerator::IsNearSea(int wx, int wz) {
	static const int dirs[8][2] = {
		{ 1, 0}, {-1, 0}, {0, 1}, {0,-1},
		{ 1, 1}, { 1,-1}, {-1, 1}, {-1,-1}
	};

	for (auto& d : dirs) {

		int nh = GetSurfaceHeight(wx + d[0], wz + d[1]);
		if (nh <= kSeaLevel) return true;

	}

	return false;

}