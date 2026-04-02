#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "block.h"
#include <stdint.h>
#include <vector>
#include <iostream>


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

	static constexpr int CHUNK_HEIGHT = 256;
	static constexpr int CHUNK_WIDTH = 16;
	static constexpr int CHUNK_SIZE = CHUNK_HEIGHT * CHUNK_WIDTH * CHUNK_WIDTH;

	unsigned int blocks[CHUNK_SIZE]{};

	std::vector<float> vertices;
	unsigned int vao, vbo = 0;
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
	bool Set(int x, int y, int z, unsigned int block) {
		if (!inRange(x, y, z)) return false;
		blocks[Index(x, y, z)] = block;
	}

};