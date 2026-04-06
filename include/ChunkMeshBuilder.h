#pragma once
#include <vector>
#include "block.h"
#include "chunk.h"


class ChunkMeshBuilder {
public:
	static void BuildMesh(Chunk* c);

private:


	static UVRect AtlasUV(int tx, int ty) {
		const float du = 1.0f / Chunk::ATLAS_COLS;
		const float dv = 1.0f / Chunk::ATLAS_ROWS;

		int flippedTy = Chunk::ATLAS_ROWS - 1 - ty;

		const float epsU = du * 0.08f;
		const float epsV = dv * 0.08f;

		float u0 = tx * du + epsU;
		float v0 = flippedTy * dv + epsV;
		float u1 = (tx + 1) * du - epsU;
		float v1 = (flippedTy + 1) * dv - epsV;

		return { u0, v0, u1, v1 };
	}

	static UVRect GetBlockUV(unsigned int block, FaceType face) {

		const unsigned int GRASS = static_cast<unsigned int>(BlockType::Grass);
		const unsigned int DIRT = static_cast<unsigned int>(BlockType::Dirt);
		const unsigned int STONE = static_cast<unsigned int>(BlockType::Stone);
		const unsigned int ORE = static_cast<unsigned int>(BlockType::Ore);
		const unsigned int WOOD = static_cast<unsigned int>(BlockType::Wood);
		const unsigned int LEAVE = static_cast<unsigned int>(BlockType::Leave);


		if (block == GRASS) {

			if (face == FaceType::Top) return AtlasUV(2, 0);
			if (face == FaceType::Bottom) return AtlasUV(18, 1);

			return AtlasUV(6, 0);

		}

		if (block == DIRT) {
			return AtlasUV(18, 1); // dirt
		}

		if (block == STONE) {
			return AtlasUV(19, 0); // stone
		}

		if (block == ORE) {
			return AtlasUV(0, 4);
		}

		if (block == WOOD) {
			if (face == FaceType::Side) return AtlasUV(3, 3);

			return AtlasUV(4, 3);
		}

		if (block == LEAVE) {
			return AtlasUV(7, 5);
		}

		return AtlasUV(1, 0);
	}

	static void AddVertex(std::vector<float>& v,
		float x, float y, float z,
		float u, float vv)
	{
		v.push_back(x);
		v.push_back(y);
		v.push_back(z);
		v.push_back(u);
		v.push_back(vv);
	}



	static void AddFaceUVFlippedX(std::vector<float>& v,
		float x0, float y0, float z0,
		float x1, float y1, float z1,
		float x2, float y2, float z2,
		float x3, float y3, float z3,
		float u0, float v0,
		float u1, float v1)
	{
		AddVertex(v, x0, y0, z0, u1, v0);
		AddVertex(v, x1, y1, z1, u0, v0);
		AddVertex(v, x2, y2, z2, u0, v1);

		AddVertex(v, x2, y2, z2, u0, v1);
		AddVertex(v, x3, y3, z3, u1, v1);
		AddVertex(v, x0, y0, z0, u1, v0);
	}

	static void AddFaceUVRotLeft90(std::vector<float>& v,
		float x0, float y0, float z0,
		float x1, float y1, float z1,
		float x2, float y2, float z2,
		float x3, float y3, float z3,
		float u0, float v0,
		float u1, float v1)
	{
		AddVertex(v, x0, y0, z0, u1, v0);
		AddVertex(v, x1, y1, z1, u1, v1);
		AddVertex(v, x2, y2, z2, u0, v1);

		AddVertex(v, x2, y2, z2, u0, v1);
		AddVertex(v, x3, y3, z3, u0, v0);
		AddVertex(v, x0, y0, z0, u1, v0);
	}

	static void AddFaceUV(std::vector<float>& v,
		float x0, float y0, float z0,
		float x1, float y1, float z1,
		float x2, float y2, float z2,
		float x3, float y3, float z3,
		float u0, float v0,
		float u1, float v1)
	{
		AddVertex(v, x0, y0, z0, u0, v0);
		AddVertex(v, x1, y1, z1, u1, v0);
		AddVertex(v, x2, y2, z2, u1, v1);

		AddVertex(v, x2, y2, z2, u1, v1);
		AddVertex(v, x3, y3, z3, u0, v1);
		AddVertex(v, x0, y0, z0, u0, v0);
	}

};