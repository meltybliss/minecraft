#include "chunk.h"

void Chunk::generate() {
	for (int y = 0; y < CHUNK_HEIGHT; y++) {
		for (int z = 0; z < CHUNK_WIDTH; z++) {
			for (int x = 0; x < CHUNK_WIDTH; x++) {
				BlockType b = BlockType::AIR;

				static int ground = CHUNK_HEIGHT / 2;

				if (y == ground) {
					b = BlockType::Grass;
				}
				else if (y < ground && y > ground - 6) {
					b = BlockType::Dirt;
				}
				else if (y <= ground - 6) {
					b = BlockType::Stone;
				}

				blocks[Index(x, y, z)] = static_cast<unsigned int>(b);
			}
		}
	}

}