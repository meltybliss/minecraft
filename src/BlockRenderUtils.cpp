#include "BlockRenderUtils.h"
#include "block.h"

UVRect BlockRenderUtils::AtlasUV(int tx, int ty) {
	const float du = 1.0f / ATLAS_COLS;
	const float dv = 1.0f / ATLAS_ROWS;

	int flippedTy = ATLAS_ROWS - 1 - ty;

	const float epsU = du * 0.08f;
	const float epsV = dv * 0.08f;

	float u0 = tx * du + epsU;
	float v0 = flippedTy * dv + epsV;
	float u1 = (tx + 1) * du - epsU;
	float v1 = (flippedTy + 1) * dv - epsV;

	return { u0, v0, u1, v1 };
}


UVRect BlockRenderUtils::GetBlockUV(unsigned int block, FaceType face) {

	const unsigned int GRASS = static_cast<unsigned int>(BlockType::Grass);
	const unsigned int DIRT = static_cast<unsigned int>(BlockType::Dirt);
	const unsigned int STONE = static_cast<unsigned int>(BlockType::Stone);
	const unsigned int ORE = static_cast<unsigned int>(BlockType::Ore);
	const unsigned int WOOD = static_cast<unsigned int>(BlockType::Wood);
	const unsigned int LEAVE = static_cast<unsigned int>(BlockType::Leave);
	const unsigned int TNT = static_cast<unsigned int>(BlockType::TNT);


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

	if (block == TNT) {
		if (face == FaceType::Top) return AtlasUV(31, 1);
		if (face == FaceType::Bottom) return AtlasUV(0, 2);

		return AtlasUV(30, 1);
	}

	return AtlasUV(1, 0);
}


void BlockRenderUtils::AddVertex(std::vector<float>& v,
	float x, float y, float z,
	float u, float vv)
{
	v.push_back(x);
	v.push_back(y);
	v.push_back(z);
	v.push_back(u);
	v.push_back(vv);
}


void BlockRenderUtils::AddFaceUVFlippedX(std::vector<float>& v,
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


void BlockRenderUtils::AddFaceUVRotLeft90(std::vector<float>& v,
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


void BlockRenderUtils::AddFaceUV(std::vector<float>& v,
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


void BlockRenderUtils::AppendBlockCube(std::vector<float>& v,
	float bx, float by, float bz,
	unsigned int block)
{
	const float x0 = bx;
	const float y0 = by;
	const float z0 = bz;

	const float x1 = bx + 1.0f;
	const float y1 = by + 1.0f;
	const float z1 = bz + 1.0f;

	UVRect top = GetBlockUV(block, FaceType::Top);
	UVRect bottom = GetBlockUV(block, FaceType::Bottom);
	UVRect side = GetBlockUV(block, FaceType::Side);

	// Top
	AddFaceUVRotLeft90(v,
		x0, y1, z0,
		x1, y1, z0,
		x1, y1, z1,
		x0, y1, z1,
		top.u0, top.v0, top.u1, top.v1);

	// Bottom
	AddFaceUV(v,
		x0, y0, z1,
		x1, y0, z1,
		x1, y0, z0,
		x0, y0, z0,
		bottom.u0, bottom.v0, bottom.u1, bottom.v1);

	// Front
	AddFaceUV(v,
		x0, y0, z1,
		x1, y0, z1,
		x1, y1, z1,
		x0, y1, z1,
		side.u0, side.v0, side.u1, side.v1);

	// Back
	AddFaceUVFlippedX(v,
		x1, y0, z0,
		x0, y0, z0,
		x0, y1, z0,
		x1, y1, z0,
		side.u0, side.v0, side.u1, side.v1);

	// Left
	AddFaceUV(v,
		x0, y0, z0,
		x0, y0, z1,
		x0, y1, z1,
		x0, y1, z0,
		side.u0, side.v0, side.u1, side.v1);

	// Right
	AddFaceUVFlippedX(v,
		x1, y0, z1,
		x1, y0, z0,
		x1, y1, z0,
		x1, y1, z1,
		side.u0, side.v0, side.u1, side.v1);
}