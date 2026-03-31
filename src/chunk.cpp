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

	float s = static_cast<float>(blockSize);

	for (int y = 0; y < CHUNK_HEIGHT; y++) {
		for (int x = 0; x < CHUNK_WIDTH; x++) {
			for (int z = 0; z < CHUNK_WIDTH; z++) {
				unsigned int block = Get(x, y, z);
				if (block == 0) continue;

				float fx = static_cast<float>(x + cx * CHUNK_WIDTH) * s;
				float fy = static_cast<float>(y) * s;
				float fz = static_cast<float>(z + cz * CHUNK_WIDTH) * s;
				//è„Ç™ãÛãCÇ»ÇÁ top face Çí«â¡

				if (x == 8 && y == CHUNK_HEIGHT / 2 - 1 && z == 8) {
					std::cout << "top? " << (Get(x, y + 1, z) == 0) << "\n";
					std::cout << "bottom? " << (Get(x, y - 1, z) == 0) << "\n";
					std::cout << "right? " << (Get(x + 1, y, z) == 0) << "\n";
					std::cout << "left? " << (Get(x - 1, y, z) == 0) << "\n";
					std::cout << "back? " << (Get(x, y, z + 1) == 0) << "\n";
					std::cout << "front? " << (Get(x, y, z - 1) == 0) << "\n";
				}

				//top
				if (Get(x, y + 1, z) == 0) {
		
					AddFace(vertices,
						fx, fy + s, fz,
						fx + s, fy + s, fz,
						fx + s, fy + s, fz + s,
						fx, fy + s, fz + s);


				} 

				//bottom
				if (Get(x, y - 1, z) == 0) {
					
					AddFace(vertices,
						fx, fy, fz,
						fx, fy, fz + s,
						fx + s, fy, fz + s,
						fx + s, fy, fz);
				}

				//right
				if (Get(x + 1, y, z) == 0) {

					AddFace(vertices,
						fx + s, fy, fz,
						fx + s, fy, fz + s,
						fx + s, fy + s, fz + s,
						fx + s, fy + s, fz);

				}
				
				//left
				if (Get(x - 1, y, z) == 0) {
					AddFace(vertices,
						fx, fy, fz,
						fx, fy, fz + s,
						fx, fy + s, fz + s,
						fx, fy + s, fz);
				}

				//back
				if (Get(x, y, z + 1) == 0) {

					AddFace(vertices,
						fx, fy, fz + s,
						fx, fy + s, fz + s,
						fx + s, fy + s, fz + s,
						fx + s, fy, fz + s);
				}

				//front
				if (Get(x, y, z - 1) == 0) {

					AddFace(vertices,
						fx, fy, fz,
						fx + s, fy, fz,
						fx + s, fy + s, fz,
						fx, fy + s, fz);
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