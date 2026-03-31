#pragma once
#include "block.h"
#include <stdint.h>

struct Chunk {

	
	int32_t cx;
	int32_t cz;

	void generate();
	void render();

	bool isDirty = false;

	static constexpr int CHUNK_HEIGHT = 256;
	static constexpr int CHUNK_WIDTH = 16;
	static constexpr int CHUNK_SIZE = CHUNK_HEIGHT * CHUNK_WIDTH * CHUNK_WIDTH;

	unsigned int blocks[CHUNK_SIZE];


	int Index(int x, int y, int z) const {
		return x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_WIDTH;
	}

	unsigned int Get(int x, int y, int z) const {
		return blocks[Index(x, y, z)];
	}
};