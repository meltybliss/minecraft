#include "MathUtils.h"
#include "chunk.h"
void CarveSphere(Chunk* c, int cx, int cy, int cz, int radius) {
	int minX = std::max(0, cx - radius);
	int maxX = std::min(Chunk::CHUNK_WIDTH - 1, cx + radius);

	int minY = std::max(0, cy - radius);
	int maxY = std::min(Chunk::CHUNK_HEIGHT - 1, cy + radius);

	int minZ = std::max(0, cz - radius);
	int maxZ = std::min(Chunk::CHUNK_WIDTH - 1, cz + radius);

	for (int z = minZ; z <= maxZ; z++) {
		for (int y = minY; y <= maxY; y++) {
			for (int x = minX; x <= maxX; x++) {
				int dx = x - cx;
				int dy = y - cy;
				int dz = z - cz;

				if (dx * dx + dy * dy + dz * dz <= radius * radius) {
					c->Set(x, y, z, (unsigned int)BlockType::AIR);
				}
			}
		}
	}
}



void CarveEllipsoid(Chunk* c, int cx, int cy, int cz, int rx, int ry, int rz) {
	int minX = std::max(0, cx - rx);
	int maxX = std::min(Chunk::CHUNK_WIDTH - 1, cx + rx);

	int minY = std::max(0, cy - ry);
	int maxY = std::min(Chunk::CHUNK_HEIGHT - 1, cy + ry);

	int minZ = std::max(0, cz - rz);
	int maxZ = std::min(Chunk::CHUNK_WIDTH - 1, cz + rz);

	for (int z = minZ; z <= maxZ; z++) {
		for (int y = minY; y <= maxY; y++) {
			for (int x = minX; x <= maxX; x++) {
				float dx = static_cast<float>(x - cx);
				float dy = static_cast<float>(y - cy);
				float dz = static_cast<float>(z - cz);

				float nx = (dx * dx) / (rx * rx);
				float ny = (dy * dy) / (ry * ry);
				float nz = (dz * dz) / (rz * rz);

				if (nx + ny + nz <= 1.0f) {
					c->Set(x, y, z, static_cast<unsigned int>(BlockType::AIR));
				}
			}
		}
	}
}