#include "chunk.h"
#include "world.h"

Chunk::~Chunk() {
	if (vao != 0) glDeleteVertexArrays(1, &vao);
	if (vbo != 0) glDeleteBuffers(1, &vbo);

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

static void AddFace(std::vector<float>& v,
	float x0, float y0, float z0,
	float x1, float y1, float z1,
	float x2, float y2, float z2,
	float x3, float y3, float z3)
{
	// triangle 1
	AddVertex(v, x0, y0, z0, 0.0f, 0.0f);
	AddVertex(v, x1, y1, z1, 1.0f, 0.0f);
	AddVertex(v, x2, y2, z2, 1.0f, 1.0f);

	// triangle 2
	AddVertex(v, x2, y2, z2, 1.0f, 1.0f);
	AddVertex(v, x3, y3, z3, 0.0f, 1.0f);
	AddVertex(v, x0, y0, z0, 0.0f, 0.0f);
}


void Chunk::generate() {
	for (int y = 0; y < CHUNK_HEIGHT; y++) {
		for (int z = 0; z < CHUNK_WIDTH; z++) {
			for (int x = 0; x < CHUNK_WIDTH; x++) {
				BlockType b = BlockType::AIR;

				int ground = CHUNK_HEIGHT / 2;

				if (y == ground) {
					b = BlockType::Grass;
				}
				else if (y < ground && y > ground - 6) {
					b = BlockType::Dirt;
				}
				else if (y <= ground - 6) {
					b = BlockType::Stone;
				}

				blocks[Index(x, y, z)] = static_cast<unsigned int>(b);
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
		
					AddFace(vertices,
						fx, fy + s, fz,
						fx + s, fy + s, fz,
						fx + s, fy + s, fz + s,
						fx, fy + s, fz + s);


				} 

				//bottom
				if (InAir(x, y - 1, z)) {
					
					AddFace(vertices,
						fx, fy, fz,
						fx, fy, fz + s,
						fx + s, fy, fz + s,
						fx + s, fy, fz);
				}

				//right
				if (InAir(x + 1, y, z)) {

					AddFace(vertices,
						fx + s, fy, fz,
						fx + s, fy, fz + s,
						fx + s, fy + s, fz + s,
						fx + s, fy + s, fz);

				}
				
				//left
				if (InAir(x - 1, y, z)) {
					AddFace(vertices,
						fx, fy, fz,
						fx, fy, fz + s,
						fx, fy + s, fz + s,
						fx, fy + s, fz);
				}

				//back
				if (InAir(x, y, z + 1)) {

					AddFace(vertices,
						fx, fy, fz + s,
						fx, fy + s, fz + s,
						fx + s, fy + s, fz + s,
						fx + s, fy, fz + s);
				}

				//front
				if (InAir(x, y, z - 1)) {

					AddFace(vertices,
						fx, fy, fz,
						fx + s, fy, fz,
						fx + s, fy + s, fz,
						fx, fy + s, fz);
				}
			}
		}
	}

	vertexCount = static_cast<int>(vertices.size() / 5);

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