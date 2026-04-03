#include "chunk.h"
#include "world.h"

Chunk::~Chunk() {
	if (vao != 0) glDeleteVertexArrays(1, &vao);
	if (vbo != 0) glDeleteBuffers(1, &vbo);

}

uint32_t Chunk::makeChunkSeed(uint32_t worldSeed, int cx, int cz) {
	uint32_t x = static_cast<uint32_t>(cx) * 73856093u;
	uint32_t z = static_cast<uint32_t>(cz) * 19349663u;
	return worldSeed ^ x ^ z;
}


void Chunk::CarveSphere(int cx, int cy, int cz, int radius) {
	for (int z = 0; z < CHUNK_WIDTH; z++) {
		for (int y = 0; y < CHUNK_HEIGHT; y++) {
			for (int x = 0; x < CHUNK_WIDTH; x++) {
				int dx = x - cx;
				int dy = y - cy;
				int dz = z - cz;

				if (dx * dx + dy * dy + dz * dz <= radius * radius) {
					Set(x, y, z, (unsigned int)BlockType::AIR);
				}


			}
		}
	}

}

#pragma region texture
static UVRect AtlasUV(int tx, int ty) {
	const float du = 1.0f / Chunk::ATLAS_COLS;
	const float dv = 1.0f / Chunk::ATLAS_ROWS;

	int flippedTy = Chunk::ATLAS_ROWS - 1 - ty;

	const float epsU = du * 0.08f;
	const float epsV = dv * 0.08f;

	float u0 = tx * du + epsU;
	float v0 = flippedTy * dv + epsV;
	float u1 = (tx + 1) * du - epsU;
	float v1 = (flippedTy + 1) * dv - epsV;

	return { u0, v0, u1, v1 };
}

static UVRect GetBlockUV(unsigned int block, FaceType face) {
	
	const unsigned int GRASS = static_cast<unsigned int>(BlockType::Grass);
	const unsigned int DIRT = static_cast<unsigned int>(BlockType::Dirt);
	const unsigned int STONE = static_cast<unsigned int>(BlockType::Stone);
	const unsigned int ORE = static_cast<unsigned int>(BlockType::Ore);

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

	return AtlasUV(1, 0);
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

static void AddFaceUVFlippedX(std::vector<float>& v,
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

static void AddFaceUVRotLeft90(std::vector<float>& v,
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
#pragma endregion texture



#pragma region ChunkGenerationFuncs 
void Chunk::FillTerrain() {
	
	for (int z = 0; z < CHUNK_WIDTH; z++) {
		for (int x = 0; x < CHUNK_WIDTH; x++) {
			int wx = cx * CHUNK_WIDTH + x;
			int wz = cz * CHUNK_WIDTH + z;


			int surfaceY = GetSurfaceHeight(wx, wz);

			for (int y = 0; y < CHUNK_HEIGHT; y++) {
				BlockType b = BlockType::AIR;

				if (y == surfaceY) {
					b = BlockType::Grass;
				}
				else if (y < surfaceY && y >= surfaceY - 4) {
					b = BlockType::Dirt;
				}
				else if (y < surfaceY - 4) {
					b = BlockType::Stone;
				}


				blocks[Index(x, y, z)] = static_cast<unsigned int>(b);
			}
		}
	}
	
}

void Chunk::GenerateStoneBlobs(std::mt19937& rng, int ground) {
	//mix stones into chunk

	std::uniform_int_distribution<int> xDist(0, CHUNK_WIDTH - 1);
	std::uniform_int_distribution<int> zDist(0, CHUNK_WIDTH - 1);
	std::uniform_int_distribution<int> stoneYDist(0, ground - 4);
	std::uniform_int_distribution<int> radiusDist(2, 4);

	for (int i = 0; i < STONE_BLOB_COUNT; i++) {

		int centerX = xDist(rng);
		int centerY = stoneYDist(rng);
		int centerZ = zDist(rng);

		int radius = radiusDist(rng);

		for (int z = 0; z < CHUNK_WIDTH; z++) {
			for (int y = 0; y < CHUNK_HEIGHT; y++) {
				for (int x = 0; x < CHUNK_WIDTH; x++) {

					int dx = x - centerX;
					int dy = y - centerY;
					int dz = z - centerZ;

					if (dx * dx + dy * dy + dz * dz <= radius * radius) {
						if (this->Get(x, y, z) != 0) {
							this->Set(x, y, z, static_cast<unsigned int>(BlockType::Stone));
						}
					}

				}
			}
		}

	}
}

void Chunk::ScatterOre(std::mt19937& rng, int ground) {
	//slitly mix ore into mass of stones
	std::uniform_int_distribution<int> chance1000(0, 999);

	for (int z = 0; z < CHUNK_WIDTH; z++) {
		for (int y = 0; y < CHUNK_HEIGHT; y++) {
			for (int x = 0; x < CHUNK_WIDTH; x++) {

				if (this->Get(x, y, z) == static_cast<unsigned int>(BlockType::Stone)) {
					if (chance1000(rng) < ORE_SCATTER_PER_1000) {//2%
						this->Set(x, y, z, (unsigned int)BlockType::Ore);
					}
				}
			}
		}
	}
}

void Chunk::GenerateOreVein(std::mt19937& rng, int ground) {
	//ore vein RandomWalk
	std::uniform_int_distribution<int> xDist(0, CHUNK_WIDTH - 1);
	std::uniform_int_distribution<int> zDist(0, CHUNK_WIDTH - 1);
	std::uniform_int_distribution<int> oreStartYDist(3, ground - 4);
	std::uniform_int_distribution<int> dirDist(0, 5);


	{
		int x = xDist(rng);
		int y = oreStartYDist(rng);
		int z = zDist(rng);

		for (int i = 0; i < ORE_VEIN_STEPS; i++) {
			if (Get(x, y, z) == (unsigned int)BlockType::Stone) {
				Set(x, y, z, (unsigned int)BlockType::Ore);
			}

			int dir = dirDist(rng);
			if (dir == 0) x++;
			if (dir == 1) x--;
			if (dir == 2) y++;
			if (dir == 3) y--;
			if (dir == 4) z++;
			if (dir == 5) z--;

			x = std::clamp(x, 0, CHUNK_WIDTH - 1);
			y = std::clamp(y, 0, CHUNK_HEIGHT - 1);
			z = std::clamp(z, 0, CHUNK_WIDTH - 1);

		}

	}
}


void Chunk::GenerateCave(std::mt19937& rng, int ground) {

	//cave RandomWalk
	std::uniform_int_distribution<int> xDist(0, CHUNK_WIDTH - 1);
	std::uniform_int_distribution<int> zDist(0, CHUNK_WIDTH - 1);
	std::uniform_int_distribution<int> dirDist(0, 5);
	std::uniform_int_distribution<int> caveStartYDist(8, ground - 1);
	{
		int x = xDist(rng);
		int y = caveStartYDist(rng);
		int z = zDist(rng);

		for (int i = 0; i < CAVE_STEPS; i++) {
			CarveSphere(x, y, z, CAVE_RADIUS);

			int dir = dirDist(rng);
			if (dir == 0) x++;
			if (dir == 1) x--;
			if (dir == 2) y++;
			if (dir == 3) y--;
			if (dir == 4) z++;
			if (dir == 5) z--;

			x = std::clamp(x, 0, CHUNK_WIDTH - 1);
			y = std::clamp(y, 0, CHUNK_HEIGHT - 1);
			z = std::clamp(z, 0, CHUNK_WIDTH - 1);

		}

	}
}
#pragma endregion ChunkGenerationFuncs

void Chunk::generate() {
	uint32_t chunkSeed = makeChunkSeed(gWorld->getWorldSeed(), cx, cz);
	std::mt19937 rng(chunkSeed);

	int ground = CHUNK_HEIGHT / 2;

	FillTerrain();
	/*GenerateStoneBlobs(rng, ground);
	ScatterOre(rng, ground);
	GenerateOreVein(rng, ground);
	GenerateCave(rng, ground);*/

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

				//上が空気なら top face を追加

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

				// right
				if (InAir(x + 1, y, z)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUVFlippedX(vertices,
						fx + s, fy, fz,
						fx + s, fy, fz + s,
						fx + s, fy + s, fz + s,
						fx + s, fy + s, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				// left
				if (InAir(x - 1, y, z)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUV(vertices,
						fx, fy, fz,
						fx, fy, fz + s,
						fx, fy + s, fz + s,
						fx, fy + s, fz,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				// back
				if (InAir(x, y, z + 1)) {
					UVRect uv = GetBlockUV(block, FaceType::Side);
					AddFaceUVRotLeft90(vertices,
						fx, fy, fz + s,
						fx, fy + s, fz + s,
						fx + s, fy + s, fz + s,
						fx + s, fy, fz + s,
						uv.u0, uv.v0, uv.u1, uv.v1);
				}

				// front
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



int Chunk::GetSurfaceHeight(int wx, int wz) const {

	float scale = 0.01f;
	float n = FractalNoise2D(wx * scale, wz * scale, gWorld->getWorldSeed());

	// 0.5 を引いてから 2倍することで、-1.0 ~ 1.0 の範囲にする
	// これで baseHeight より低い「谷」も作られるようになります
	float heightOffset = (n - 0.5f) * 2.0f;

	int baseHeight = CHUNK_HEIGHT / 2;
	// 山を高くしたいならここを 20 ~ 30 に上げる
	int amplitude = 20;

	return baseHeight + (int)std::round(heightOffset * amplitude);
}