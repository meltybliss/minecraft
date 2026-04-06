#pragma once
#include "chunk.h"
#include "CaveSeed.h"
#include "GetSurfaceHeight.h"


class World;

class CaveGenerator {
public:

	static void ApplyCaves(Chunk* c);

private:
	static void ApplyCavesFromSourceChunk(Chunk* c, int scx, int scz);

	static void ApplySingleCave(Chunk* c, const CaveSeed& cave, int depth);

	static std::vector<CaveSeed> BuildCavesFromSourceChunk(Chunk* c, int scx, int scz);
};