#include "ChunkMeshBuilder.h"
#include "world.h"

void ChunkMeshBuilder::BuildMesh(Chunk* c) {
	c->vertices.clear();
	c->waterVertices.clear();

	float s = static_cast<float>(blockSize);

	Chunk* neighbors[3][3];
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			neighbors[i + 1][j + 1] = gWorld->GetChunkPtr(c->cx + i, c->cz + j);
		}
	}

	for (int y = 0; y < Chunk::CHUNK_HEIGHT; y++) {
		for (int x = 0; x < Chunk::CHUNK_WIDTH; x++) {
			for (int z = 0; z < Chunk::CHUNK_WIDTH; z++) {


				unsigned int block = c->Get(x, y, z);
				if (block == 0) continue;

				float fx = static_cast<float>(x + c->cx * Chunk::CHUNK_WIDTH) * s;
				float fy = static_cast<float>(y) * s;
				float fz = static_cast<float>(z + c->cz * Chunk::CHUNK_WIDTH) * s;

				auto& outVerts = block == (unsigned int)BlockType::Water ?
					c->waterVertices : c->vertices;

				auto GetNeighborBlock = [&](int nx, int ny, int nz) -> unsigned int {
					if (ny < 0 || ny >= Chunk::CHUNK_HEIGHT) {
						return (unsigned int)BlockType::AIR;
					}

					int targetCx = 1;
					int targetCz = 1;
					int lx = nx, lz = nz;

					if (nx < 0) { targetCx = 0; lx = Chunk::CHUNK_WIDTH - 1; }
					else if (nx >= Chunk::CHUNK_WIDTH) { targetCx = 2; lx = 0; }

					if (nz < 0) { targetCz = 0; lz = Chunk::CHUNK_WIDTH - 1; }
					else if (nz >= Chunk::CHUNK_WIDTH) { targetCz = 2; lz = 0; }

					Chunk* target = neighbors[targetCx][targetCz];
					if (target == nullptr) return (unsigned int)BlockType::Dirt;
					if (!target->isGenerated) return (unsigned int)BlockType::Dirt;

					return target->Get(lx, ny, lz);
				};

				auto IsWater = [&](unsigned int b) -> bool {
					return b == (unsigned int)BlockType::Water;
				};

				auto IsAir = [&](unsigned int b) -> bool {
					return b == (unsigned int)BlockType::AIR;
				};


				auto ShouldDrawFace = [&](unsigned int self, unsigned int neighbor) -> bool {
					
					bool selfLiq = IsWater(self);
					bool neighborAir = IsAir(neighbor);
					bool neighborLiq = IsWater(neighbor);

					if (!selfLiq) {
						return neighborAir || neighborLiq;
					}

					return neighborAir;
				};


				auto self = block;
				auto top = GetNeighborBlock(x, y + 1, z);
				auto bottom = GetNeighborBlock(x, y - 1, z);
				auto right = GetNeighborBlock(x + 1, y, z);
				auto left = GetNeighborBlock(x - 1, y, z);
				auto back = GetNeighborBlock(x, y, z + 1);
				auto front = GetNeighborBlock(x, y, z - 1);

				//top
				if (ShouldDrawFace(self, top)) {
					
					UVRect uv = GetBlockUV(block, FaceType::Top);

					AddFaceUV(outVerts,
						fx, fy + s, fz,
						fx + s, fy + s, fz,
						fx + s, fy + s, fz + s,
						fx, fy + s, fz + s,
						uv.u0, uv.v0, uv.u1, uv.v1);


				}

				//bottom
				if (ShouldDrawFace(self, bottom)) {
					UVRect uv = GetBlockUV(block, FaceType::Bottom);
					AddFaceUV(outVerts,
						fx, fy, fz,
						fx, fy, fz + s,
						fx + s, fy, fz + s,
						fx + s, fy, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				// right
				if (ShouldDrawFace(self, right)) {

					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUVFlippedX(outVerts,
						fx + s, fy, fz,
						fx + s, fy, fz + s,
						fx + s, fy + s, fz + s,
						fx + s, fy + s, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				// left
				if (ShouldDrawFace(self, left)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUV(outVerts,
						fx, fy, fz,
						fx, fy, fz + s,
						fx, fy + s, fz + s,
						fx, fy + s, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				// back
				if (ShouldDrawFace(self, back)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUVRotLeft90(outVerts,
						fx, fy, fz + s,
						fx, fy + s, fz + s,
						fx + s, fy + s, fz + s,
						fx + s, fy, fz + s,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				// front
				if (ShouldDrawFace(self, front)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUV(outVerts,
						fx, fy, fz,
						fx + s, fy, fz,
						fx + s, fy + s, fz,
						fx, fy + s, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}
			}
		}
	}


	if (c->vertices.empty() && c->waterVertices.empty()) {
		c->vertexCount = 0;
		c->waterVertexCount = 0;
		return;
	}

	c->vertexCount = static_cast<int>(c->vertices.size() / 5);


	if (c->vao == 0) glGenVertexArrays(1, &c->vao);
	if (c->vbo == 0) glGenBuffers(1, &c->vbo);

	glBindVertexArray(c->vao);
	glBindBuffer(GL_ARRAY_BUFFER, c->vbo);
	glBufferData(GL_ARRAY_BUFFER, c->vertices.size() * sizeof(float), c->vertices.data(), GL_STATIC_DRAW);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// uv
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	c->waterVertexCount = static_cast<int>(c->waterVertices.size() / 5);

	if (c->waterVAO == 0) glGenVertexArrays(1, &c->waterVAO);
	if (c->waterVBO == 0) glGenBuffers(1, &c->waterVBO);

	glBindVertexArray(c->waterVAO);
	glBindBuffer(GL_ARRAY_BUFFER, c->waterVBO);
	glBufferData(GL_ARRAY_BUFFER, c->waterVertices.size() * sizeof(float), c->waterVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	glBindVertexArray(0);

	c->isDirty = false;
	c->isEdited = false;
}