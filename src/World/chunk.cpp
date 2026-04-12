#include "World/chunk.h"
#include "World/world.h"


Chunk::Chunk() {

	
}

Chunk::~Chunk() {
	
	if (vao != 0 && vbo != 0) {

		gWorld->EnqueueGpuDelete(vao, vbo);
		vao = 0;
		vbo = 0;
	}
}
uint32_t Chunk::makeChunkSeed(uint32_t worldSeed, int cx, int cz) {
	uint32_t x = static_cast<uint32_t>(cx) * 73856093u;
	uint32_t z = static_cast<uint32_t>(cz) * 19349663u;
	return worldSeed ^ x ^ z;
}

uint64_t Chunk::GetChunkKey(int32_t scx, int32_t scz) {
	return (static_cast<uint64_t>(static_cast<uint32_t>(scx)) << 32) |
		static_cast<uint32_t>(scz);
}


void Chunk::renderSolid(GLuint program) {
	if (vertexCount <= 0) return;

	GLint alphaLoc = glGetUniformLocation(program, "uAlpha");

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glUniform1f(alphaLoc, 1.0f);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	glBindVertexArray(0);
}

void Chunk::renderWater(GLuint program) {
	if (waterVertexCount <= 0) return;

	GLint alphaLoc = glGetUniformLocation(program, "uAlpha");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glUniform1f(alphaLoc, 0.5f);

	glBindVertexArray(waterVAO);
	glDrawArrays(GL_TRIANGLES, 0, waterVertexCount);
	glBindVertexArray(0);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}


void Chunk::RebuildSkyLight() {
	if (!isLightDirty) return;

	for (auto& b : blocks) {
		b.skyLight = 0;
	}

	struct LightNode {
		int x, y, z;
	};

	std::queue<LightNode> q;

	auto IsTransparent = [&](const unsigned int b) -> bool {
		return b == 0 || b == (unsigned int)BlockType::Leave || b == (unsigned int)BlockType::Water;
	};

	auto IsAir = [&](const unsigned int b) -> bool {
		return b == 0;
	};

	auto InBounds = [&](int x, int y, int z) -> bool {
		return { x >= 0 && x < CHUNK_WIDTH &&
				y >= 0 && y < CHUNK_HEIGHT &&
				z >= 0 && z < CHUNK_WIDTH };

	};

	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int z = 0; z < CHUNK_WIDTH; z++) {

			for (int y = CHUNK_HEIGHT - 1; y >= 0; y--) {

				unsigned int block = Get(x, y, z);
				
				if (IsAir(block)) {
					int index = Index(x, y, z);
					blocks[index].skyLight = 15;
					q.push({ x, y, z });
				}
				else {

					break;
				}

			}


		}
	}


	while (!q.empty()) {
		LightNode cur = q.front();
		q.pop();

		int curIdx = Index(cur.x, cur.y, cur.z);
		uint8_t curLight = blocks[curIdx].skyLight;

		if (curLight == 0) continue;

		//right, left, front, back, down
		const int dirs[5][3] = {
			{1, 0, 0},
			{-1, 0, 0},
			{0, 0, 1},
			{0, 0, -1},
			{0, -1, 0},

		};

		for (const auto& dir : dirs) {
			int nx = cur.x + dir[0];
			int ny = cur.y + dir[1];
			int nz = cur.z + dir[2];

			if (!InBounds(nx, ny, nz)) continue;

			int nIdx = Index(nx, ny, nz);
			unsigned int nBlock = blocks[nIdx].type;

			if (!IsTransparent(nBlock)) continue;

			uint8_t newLight = 0;

			if (dir[0] == 0 && dir[1] == -1 && dir[2] == 0 && curLight == 15) {

				newLight = curLight;
			}
			else {
				if (curLight <= 1) continue;

				if (IsAir(nBlock)) {

					newLight = curLight - 1;
				}
				else {
					newLight = curLight - 2;
				}
			}

			if (newLight > blocks[nIdx].skyLight) {
				blocks[nIdx].skyLight = newLight;
				q.push({ nx, ny, nz });
			}

		}


	}


	isLightDirty = false;
}