#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <vector>
struct UVRect {
	float u0, v0, u1, v1;
};


enum class FaceType {
	Top,
	Bottom,
	Side
};


namespace BlockRenderUtils {
	UVRect AtlasUV(int tx, int ty);
	UVRect GetBlockUV(unsigned int block, FaceType face);
	void AddVertex(std::vector<float>& v,
		float x, float y, float z,
		float u, float vv);

	void AddFaceUVFlippedX(std::vector<float>& v,
		float x0, float y0, float z0,
		float x1, float y1, float z1,
		float x2, float y2, float z2,
		float x3, float y3, float z3,
		float u0, float v0,
		float u1, float v1);


	void AddFaceUVRotLeft90(std::vector<float>& v,
		float x0, float y0, float z0,
		float x1, float y1, float z1,
		float x2, float y2, float z2,
		float x3, float y3, float z3,
		float u0, float v0,
		float u1, float v1);


	void AddFaceUV(std::vector<float>& v,
		float x0, float y0, float z0,
		float x1, float y1, float z1,
		float x2, float y2, float z2,
		float x3, float y3, float z3,
		float u0, float v0,
		float u1, float v1);

	void AppendBlockCube(std::vector<float>& v,
		float bx, float by, float bz,
		unsigned int block);


	constexpr int ATLAS_COLS = 32;
	constexpr int ATLAS_ROWS = 16;
}