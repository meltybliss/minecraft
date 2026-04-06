#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "block.h"
#include <stdint.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>
#include "noise.h"
#include "CaveSeed.h"
#include <unordered_map>

struct UVRect {
	float u0, v0, u1, v1;
};


enum class FaceType {
	Top,
	Bottom,
	Side
};


struct Chunk {

	Chunk();
	~Chunk();


	int32_t cx = 0;
	int32_t cz = 0;

	uint32_t chunkSeed = 0;

	void render();

	bool isGenerated = false;
	bool isEdited = false;
	bool isDirty = false;
	bool isQueuedForGen = false;
	bool isQueuedForMesh = false;
	bool isQueuedForUnload = false;

	static constexpr int CHUNK_HEIGHT = 356;//256
	static constexpr int CHUNK_WIDTH = 16;
	static constexpr int CHUNK_SIZE = CHUNK_HEIGHT * CHUNK_WIDTH * CHUNK_WIDTH;

	unsigned int blocks[CHUNK_SIZE]{};

	std::vector<float> vertices;
	unsigned int vao = 0;
	unsigned int vbo = 0;
	int vertexCount = 0;

	bool inRange(int x, int y, int z) const {
		return 
			x >= 0 && x < CHUNK_WIDTH &&
			y >= 0 && y < CHUNK_HEIGHT &&
			z >= 0 && z < CHUNK_WIDTH;
	}

	int Index(int x, int y, int z) const {
		return x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_WIDTH;
	}

	unsigned int Get(int x, int y, int z) const {
		if (!inRange(x, y, z)) return 0;
		return blocks[Index(x, y, z)];
	}

	bool isAirBlock(int x, int y, int z) const {
		return (this->Get(x, y, z) == 0);
	}

	bool Set(int x, int y, int z, unsigned int block) {
		if (!inRange(x, y, z)) return false;

		blocks[Index(x, y, z)] = block;

		return true;
	}


	static uint32_t makeChunkSeed(uint32_t worldSeed, int cx, int cz);
	static uint64_t GetChunkKey(int32_t scx, int32_t scz);

	
	std::unordered_map<uint64_t, std::vector<CaveSeed>> cavesCashe;

	static constexpr int STONE_BLOB_COUNT = 8;
	static constexpr int ORE_SCATTER_PER_1000 = 20;
	static constexpr int ORE_VEIN_STEPS = 30;
	static constexpr int CAVE_STEPS = 50;
	static constexpr int CAVE_RADIUS = 2;

	static constexpr int ATLAS_COLS = 32;
	static constexpr int ATLAS_ROWS = 16;

	

};