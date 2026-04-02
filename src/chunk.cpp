#include "chunk.h"
#include "world.h"

Chunk::~Chunk() {
	if (vao != 0) glDeleteVertexArrays(1, &vao);
	if (vbo != 0) glDeleteBuffers(1, &vbo);

}

static UVRect AtlasUV(int tx, int ty, int cols, int rows) {
	const float du = 1.0f / cols;
	const float dv = 1.0f / rows;

	float u0 = tx * du;
	float v0 = ty * dv;
	float u1 = u0 + du;
	float v1 = v0 + dv;

	return { u0, v0, u1, v1 };
}

static UVRect GetBlockUV(unsigned int block, FaceType face) {
	
	const unsigned int DIRT = static_cast<unsigned int>(BlockType::Dirt);
	const unsigned int STONE = static_cast<unsigned int>(BlockType::Stone);

	if (block == DIRT) {
		return AtlasUV(2, 3, 4, 5); // dirt
	}

	if (block == STONE) {
		return AtlasUV(0, 0, 4, 5); // stone
	}

	return AtlasUV(0, 1, 4, 5);
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


void Chunk::generate() {

	int ground = CHUNK_HEIGHT / 2;

	for (int y = 0; y < CHUNK_HEIGHT; y++) {
		for (int z = 0; z < CHUNK_WIDTH; z++) {
			for (int x = 0; x < CHUNK_WIDTH; x++) {
				BlockType b = BlockType::AIR;

				if (y <= ground) {
					b = BlockType::Dirt;
				}
				

				blocks[Index(x, y, z)] = static_cast<unsigned int>(b);
			}
		}
	}

	for (int i = 0; i < 8; i++) {
		int cx = rand() % CHUNK_WIDTH;
		int cy = rand() % (ground + 1);
		int cz = rand() % CHUNK_WIDTH;
		int radius = 2 + rand() % 3;

		for (int z = 0; z < CHUNK_WIDTH; z++) {
			for (int y = 0; y < CHUNK_HEIGHT; y++) {
				for (int x = 0; x < CHUNK_WIDTH; x++) {

					int dx = x - cx;
					int dy = y - cy;
					int dz = z - cz;

					if (dx * dx + dy * dy + dz * dz <= radius * radius) {
						if (this->Get(x, y, z) != 0) {
							this->Set(x, y, z, static_cast<unsigned int>(BlockType::Stone));
						}
					}

				}
			}
		}

	}

	isDirty = true;

}


void Chunk::buildMesh() {
	vertices.clear();

	float s = static_cast<float>(blockSize);

	Chunk* neighbors[3][3];
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			neighbors[i + 1][j + 1] = gWorld->GetChunkPtr(cx + i, cz + j);
		}
	}

	for (int y = 0; y < CHUNK_HEIGHT; y++) {
		for (int x = 0; x < CHUNK_WIDTH; x++) {
			for (int z = 0; z < CHUNK_WIDTH; z++) {
				

				unsigned int block = Get(x, y, z);
				if (block == 0) continue;

				float fx = static_cast<float>(x + cx * CHUNK_WIDTH) * s;
				float fy = static_cast<float>(y) * s;
				float fz = static_cast<float>(z + cz * CHUNK_WIDTH) * s;

				//ã‚ª‹ó‹C‚È‚ç top face ‚ð’Ç‰Á

				auto InAir = [&](int nx, int ny, int nz) -> bool {
					if (ny < 0 || ny >= CHUNK_HEIGHT) return true;

					int targetCx = 1;
					int targetCz = 1;
					int lx = nx, lz = nz;

					if (nx < 0) { targetCx = 0; lx = CHUNK_WIDTH - 1; }
					else if (nx >= CHUNK_WIDTH) { targetCx = 2; lx = 0; }

					if (nz < 0) { targetCz = 0; lz = CHUNK_WIDTH - 1; }
					else if (nz >= CHUNK_WIDTH) { targetCz = 2; lz = 0; }

					Chunk* target = neighbors[targetCx][targetCz];
					if (target == nullptr) return true;

					return target->Get(lx, ny, lz) == 0;
				};

				//top
				if (InAir(x, y + 1, z)) {
					UVRect uv = GetBlockUV(block, FaceType::Top);

					AddFaceUV(vertices,
						fx, fy + s, fz,
						fx + s, fy + s, fz,
						fx + s, fy + s, fz + s,
						fx, fy + s, fz + s,
						uv.u0, uv.v0, uv.u1, uv.v1);


				} 

				//bottom
				if (InAir(x, y - 1, z)) {
					UVRect uv = GetBlockUV(block, FaceType::Bottom);
					AddFaceUV(vertices,
						fx, fy, fz,
						fx, fy, fz + s,
						fx + s, fy, fz + s,
						fx + s, fy, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				//right
				if (InAir(x + 1, y, z)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUV(vertices,
						fx + s, fy, fz,
						fx + s, fy, fz + s,
						fx + s, fy + s, fz + s,
						fx + s, fy + s, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);

				}
				
				//left
				if (InAir(x - 1, y, z)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUV(vertices,
						fx, fy, fz,
						fx, fy, fz + s,
						fx, fy + s, fz + s,
						fx, fy + s, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				//back
				if (InAir(x, y, z + 1)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUV(vertices,
						fx, fy, fz + s,
						fx, fy + s, fz + s,
						fx + s, fy + s, fz + s,
						fx + s, fy, fz + s,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				//front
				if (InAir(x, y, z - 1)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUV(vertices,
						fx, fy, fz,
						fx + s, fy, fz,
						fx + s, fy + s, fz,
						fx, fy + s, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}
			}
		}
	}

	vertexCount = static_cast<int>(vertices.size() / 5);


	if (vao == 0) glGenVertexArrays(1, &vao);
	if (vbo == 0) glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// uv
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	isDirty = false;

}


void Chunk::render() {

	if (isDirty) {
		buildMesh();
	}

	if (vertexCount == 0) return;

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	glBindVertexArray(0);
}
