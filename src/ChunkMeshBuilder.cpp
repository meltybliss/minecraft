#include "ChunkMeshBuilder.h"
#include "world.h"

void ChunkMeshBuilder::BuildMesh(Chunk* c) {
	c->vertices.clear();

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

				//上が空気なら top face を追加

				auto InAir = [&](int nx, int ny, int nz) -> bool {
					if (ny < 0 || ny >= Chunk::CHUNK_HEIGHT) return true;

					int targetCx = 1;
					int targetCz = 1;
					int lx = nx, lz = nz;

					if (nx < 0) { targetCx = 0; lx = Chunk::CHUNK_WIDTH - 1; }
					else if (nx >= Chunk::CHUNK_WIDTH) { targetCx = 2; lx = 0; }

					if (nz < 0) { targetCz = 0; lz = Chunk::CHUNK_WIDTH - 1; }
					else if (nz >= Chunk::CHUNK_WIDTH) { targetCz = 2; lz = 0; }

					Chunk* target = neighbors[targetCx][targetCz];
					if (target == nullptr) return false;
					if (!target->isGenerated) return false;

					return target->Get(lx, ny, lz) == 0;
					};

				//top
				if (InAir(x, y + 1, z)) {
					UVRect uv = GetBlockUV(block, FaceType::Top);

					AddFaceUV(c->vertices,
						fx, fy + s, fz,
						fx + s, fy + s, fz,
						fx + s, fy + s, fz + s,
						fx, fy + s, fz + s,
						uv.u0, uv.v0, uv.u1, uv.v1);


				}

				//bottom
				if (InAir(x, y - 1, z)) {
					UVRect uv = GetBlockUV(block, FaceType::Bottom);
					AddFaceUV(c->vertices,
						fx, fy, fz,
						fx, fy, fz + s,
						fx + s, fy, fz + s,
						fx + s, fy, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				// right
				if (InAir(x + 1, y, z)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUVFlippedX(c->vertices,
						fx + s, fy, fz,
						fx + s, fy, fz + s,
						fx + s, fy + s, fz + s,
						fx + s, fy + s, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				// left
				if (InAir(x - 1, y, z)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUV(c->vertices,
						fx, fy, fz,
						fx, fy, fz + s,
						fx, fy + s, fz + s,
						fx, fy + s, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				// back
				if (InAir(x, y, z + 1)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUVRotLeft90(c->vertices,
						fx, fy, fz + s,
						fx, fy + s, fz + s,
						fx + s, fy + s, fz + s,
						fx + s, fy, fz + s,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				// front
				if (InAir(x, y, z - 1)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUV(c->vertices,
						fx, fy, fz,
						fx + s, fy, fz,
						fx + s, fy + s, fz,
						fx, fy + s, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}
			}
		}
	}


	if (c->vertices.empty()) {
		c->vertexCount = 0;
		return; // データがないなら転送せずに終わる
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

	glBindVertexArray(0);

	c->isDirty = false;
	c->isEdited = false;
}