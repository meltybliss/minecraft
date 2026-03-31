#include "chunk.h"

static void AddFace(std::vector<float>& v,
	float x0, float y0, float z0,
	float x1, float y1, float z1,
	float x2, float y2, float z2,
	float x3, float y3, float z3) {

	//triangle 1
	v.push_back(x0); v.push_back(y0); v.push_back(z0);
	v.push_back(x1); v.push_back(y1); v.push_back(z1);
	v.push_back(x2); v.push_back(y2); v.push_back(z2);

	//triangle 2
	v.push_back(x2); v.push_back(y2); v.push_back(z2);
	v.push_back(x3); v.push_back(y3); v.push_back(z3);
	v.push_back(x0); v.push_back(y0); v.push_back(z0);


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

	for (int y = 0; y < CHUNK_HEIGHT; y++) {
		for (int x = 0; x < CHUNK_WIDTH; x++) {
			for (int z = 0; z < CHUNK_WIDTH; z++) {
				unsigned int block = Get(x, y, z);
				if (block == 0) continue;

				//ã‚ª‹ó‹C‚È‚ç top face ‚ð’Ç‰Á
				if (Get(x, y + 1, z) == 0) {
					float fx = static_cast<float>(x + cx * CHUNK_WIDTH);
					float fy = static_cast<float>(y);
					float fz = static_cast<float>(z + cz * CHUNK_WIDTH);

					AddFace(vertices,
						fx, fy + 1, fz,
						fx + 1, fy + 1, fz,
						fx + 1, fy + 1, fz + 1,
						fx, fy + 1, fz + 1);


				}
			}
		}
	}

	vertexCount = static_cast<int>(vertices.size() / 3);

	if (vao == 0) glGenVertexArrays(1, &vao);
	if (vbo == 0) glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

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