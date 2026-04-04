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

	~Chunk();

	int32_t cx;
	int32_t cz;

	void generate();
	void buildMesh();
	void render();

	bool isDirty = false;
	bool isQueuedForGen = false;
	bool isQueuedForMesh = false;

	static constexpr int CHUNK_HEIGHT = 256;
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

	void CarveSphere(int cx, int cy, int cz, int radius);
	void CarveEllipsoid(int cx, int cy, int cz, int rx, int ry, int rz);

	static uint32_t makeChunkSeed(uint32_t worldSeed, int cx, int cz);
	static uint64_t GetChunkKey(int32_t scx, int32_t scz);

	void FillTerrain();
	void GenerateTrees(std::mt19937&);
	void GenerateStoneBlobs(std::mt19937& rng, int ground);
	void ScatterOre(std::mt19937& rng, int ground);
	void GenerateOreVein(std::mt19937& rng, int ground);
	void GenerateCave(std::mt19937& rng);


	void ApplyCaves();
	void ApplyCavesFromSourceChunk(int scx, int scz);
	std::vector<CaveSeed> BuildCavesFromSourceChunk(int scx, int scz);
	void ApplySingleCave(const CaveSeed& cave, int depth);

	int GetSurfaceHeight(int wx, int wz) const;

	std::unordered_map<uint64_t, std::vector<CaveSeed>> cavesCashe;

	static constexpr int STONE_BLOB_COUNT = 8;
	static constexpr int ORE_SCATTER_PER_1000 = 20;
	static constexpr int ORE_VEIN_STEPS = 30;
	static constexpr int CAVE_STEPS = 50;
	static constexpr int CAVE_RADIUS = 2;

	static constexpr int ATLAS_COLS = 32;
	static constexpr int ATLAS_ROWS = 16;

	static constexpr int kCaveSalt = 9999u;

};