#pragma once
#include "chunk.h"
#include "GetSurfaceHeight.h"
#include <random>

class TerrainGenerator {
public:
	static void Generate(Chunk* c);

private:

	static void FillTerrain(Chunk* c);
	static void GenerateTrees(Chunk* c, std::mt19937& rng);
};