#include "chunk.h"
#include "world.h"

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
	const unsigned int WOOD = static_cast<unsigned int>(BlockType::Wood);
	const unsigned int LEAVE = static_cast<unsigned int>(BlockType::Leave);


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

	if (block == WOOD) {
		if (face == FaceType::Side) return AtlasUV(3, 3);

		return AtlasUV(4, 3);
	}

	if (block == LEAVE) {
		return AtlasUV(7, 5);
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


void Chunk::CarveSphere(int cx, int cy, int cz, int radius) {
	int minX = std::max(0, cx - radius);
	int maxX = std::min(CHUNK_WIDTH - 1, cx + radius);

	int minY = std::max(0, cy - radius);
	int maxY = std::min(CHUNK_HEIGHT - 1, cy + radius);

	int minZ = std::max(0, cz - radius);
	int maxZ = std::min(CHUNK_WIDTH - 1, cz + radius);

	for (int z = minZ; z <= maxZ; z++) {
		for (int y = minY; y <= maxY; y++) {
			for (int x = minX; x <= maxX; x++) {
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

void Chunk::CarveEllipsoid(int cx, int cy, int cz, int rx, int ry, int rz) {
	int minX = std::max(0, cx - rx);
	int maxX = std::min(CHUNK_WIDTH - 1, cx + rx);

	int minY = std::max(0, cy - ry);
	int maxY = std::min(CHUNK_HEIGHT - 1, cy + ry);

	int minZ = std::max(0, cz - rz);
	int maxZ = std::min(CHUNK_WIDTH - 1, cz + rz);

	for (int z = minZ; z <= maxZ; z++) {
		for (int y = minY; y <= maxY; y++) {
			for (int x = minX; x <= maxX; x++) {
				float dx = static_cast<float>(x - cx);
				float dy = static_cast<float>(y - cy);
				float dz = static_cast<float>(z - cz);

				float nx = (dx * dx) / (rx * rx);
				float ny = (dy * dy) / (ry * ry);
				float nz = (dz * dz) / (rz * rz);

				if (nx + ny + nz <= 1.0f) {
					Set(x, y, z, static_cast<unsigned int>(BlockType::AIR));
				}
			}
		}
	}
}

void Chunk::GenerateTrees(std::mt19937& rng) {
	std::uniform_int_distribution<int> chance(0, 99);
	std::uniform_int_distribution<int> heightDist(4, 6);


	for (int z = 2; z < CHUNK_WIDTH - 2; z++) {
		for (int x = 2; x < CHUNK_WIDTH - 2; x++) {
			if (chance(rng) >= 1) continue;

			int wx = cx * CHUNK_WIDTH + x;
			int wz = cz * CHUNK_WIDTH + z;
			int y = GetSurfaceHeight(wx, wz);

			if (Get(x, y, z) != (unsigned int)BlockType::Grass) continue;
			if (y + 6 >= CHUNK_HEIGHT) continue;

			int trunkH = heightDist(rng);

			for (int i = 1; i <= trunkH; i++) {
				Set(x, y + i, z, (unsigned int)BlockType::Wood);
			}

			int topY = y + trunkH;

			for (int dz = -2; dz <= 2; dz++) {
				for (int dx = -2; dx <= 2; dx++) {
					for (int dy = -1; dy <= 1; dy++) {
						if (std::abs(dx) == 2 && std::abs(dz) == 2 && dy == -1) continue;

						int lx = x + dx;
						int ly = topY + dy;
						int lz = z + dz;

						if (lx < 0 || lx >= CHUNK_WIDTH ||
							lz < 0 || lz >= CHUNK_WIDTH ||
							ly < 0 || ly >= CHUNK_HEIGHT) continue;

						if (Get(lx, ly, lz) == (unsigned int)BlockType::AIR) {
							Set(lx, ly, lz, (unsigned int)BlockType::Leave);
						}
					}
				}
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

void Chunk::ApplyCaves() {
	static int R = 1;

	for (int scz = cz - R; scz <= cz + R; scz++) {
		for (int scx = cx - R; scx <= cx + R; scx++) {
			ApplyCavesFromSourceChunk(scx, scz);
		}
	}



}

void Chunk::ApplyCavesFromSourceChunk(int scx, int scz) {
	
	uint64_t key = GetChunkKey(scx, scz);

	auto it = cavesCashe.find(key);
	if (it == cavesCashe.end()) {
		auto caves = BuildCavesFromSourceChunk(scx, scz);
		auto [insertedIt, _] = cavesCashe.emplace(key, std::move(caves));
		it = insertedIt;
	}

	
	for (const auto& cave : it->second) {
		ApplySingleCave(cave, 0);
	}


}

void Chunk::ApplySingleCave(const CaveSeed& cave, int depth) {
	std::mt19937 stepRng(cave.stepSeed);

	std::uniform_real_distribution<float> turnDelta(-cave.turnStrength, cave.turnStrength);
	std::uniform_real_distribution<float> riseDelta(-cave.riseStrength, cave.riseStrength);
	std::uniform_real_distribution<float> radiusDelta(-cave.radiusJitter, cave.radiusJitter);
	std::uniform_int_distribution<int> roomRoll(0, 99);
	std::uniform_int_distribution<int> branchRoll(0, 99);

	float x = cave.startX;
	float y = cave.startY;
	float z = cave.startZ;

	float yaw = cave.yaw;
	float pitch = cave.pitch;
	float radius = cave.radiusStart;

	int chunkMinX = cx * CHUNK_WIDTH;
	int chunkMaxX = chunkMinX + CHUNK_WIDTH - 1;
	int chunkMinZ = cz * CHUNK_WIDTH;
	int chunkMaxZ = chunkMinZ + CHUNK_WIDTH - 1;

	int cachedSurfaceY = GetSurfaceHeight(static_cast<int>(std::round(x)), static_cast<int>(std::round(z)));


	for (int i = 0; i < cave.steps; i++) {
		float margin = cave.radiusMax + 8.0f;
		if (x < chunkMinX - margin || x > chunkMaxX + margin ||
			z < chunkMinZ - margin || z > chunkMaxZ + margin) {
			break;
		}

		if (depth < 2 && branchRoll(stepRng) < cave.branchChancePercent) {
			std::uniform_real_distribution<float> branchAngle(0.6f, 1.2f);
			std::uniform_int_distribution<int> leftRight(0, 1);

			CaveSeed branch = cave;
			branch.startX = x;
			branch.startY = y;
			branch.startZ = z;

			float angle = branchAngle(stepRng);
			if (leftRight(stepRng) == 0) angle = -angle;

			branch.yaw = yaw + angle;
			branch.pitch = pitch * 0.5f;

			branch.steps = std::max(12, (cave.steps - i) / 2);

			branch.radiusMin = std::max(0.8f, cave.radiusMin * 0.8f);
			branch.radiusStart = std::max(branch.radiusMin, radius * 0.8f);
			branch.radiusMax = std::max(branch.radiusStart + 0.5f, cave.radiusMax * 0.85f);

			branch.roomChancePercent = std::max(0, cave.roomChancePercent - 2);
			branch.branchChancePercent = std::max(0, cave.branchChancePercent - 1);

			branch.stepSeed = stepRng();

			branch.type = CaveType::ThinBranch;

			ApplySingleCave(branch, depth + 1);
		}

		int carveRadius = std::max(1, static_cast<int>(std::round(radius)));
		
		if (x + carveRadius >= chunkMinX && x - carveRadius <= chunkMaxX &&
			z + carveRadius >= chunkMinZ && z - carveRadius <= chunkMaxZ) {

			int localX = static_cast<int>(std::round(x)) - chunkMinX;
			int localY = static_cast<int>(std::round(y));
			int localZ = static_cast<int>(std::round(z)) - chunkMinZ;

			int rx, ry, rz;

			switch (cave.type) {
			case CaveType::MainTunnel:
				rx = std::max(1, (int)std::round(radius * 1.5f));
				ry = std::max(1, (int)std::round(radius * 0.75f));
				rz = std::max(1, (int)std::round(radius * 1.2f));
				break;

			case CaveType::ThinBranch:
				rx = std::max(1, (int)std::round(radius * 1.2f));
				ry = std::max(1, (int)std::round(radius * 0.8f));
				rz = std::max(1, (int)std::round(radius * 1.0f));
				break;

			case CaveType::Roomy:
				rx = std::max(1, (int)std::round(radius * 1.8f));
				ry = std::max(1, (int)std::round(radius * 1.1f));
				rz = std::max(1, (int)std::round(radius * 1.6f));
				break;
			}

			CarveEllipsoid(localX, localY, localZ, rx, ry, rz);

			if (roomRoll(stepRng) < cave.roomChancePercent) {
				CarveEllipsoid(localX, localY, localZ, rx + 2, ry + 1, rz + 2);
			}
		}

		yaw += turnDelta(stepRng);
		pitch += riseDelta(stepRng) * 0.2f;
		pitch = std::clamp(pitch, -0.18f, 0.18f);

		float dx = std::cos(pitch) * std::cos(yaw);
		float dy = std::sin(pitch);
		float dz = std::cos(pitch) * std::sin(yaw);

		x += dx * 0.6f;
		y += dy * 0.6f;
		z += dz * 0.6f;

		if (i % 4 == 0) {
			cachedSurfaceY = GetSurfaceHeight(static_cast<int>(std::round(x)), static_cast<int>(std::round(z)));
		}

		int minY = 6;
		int maxY = std::max(minY, cachedSurfaceY - 6);
		y = std::clamp(y, static_cast<float>(minY), static_cast<float>(maxY));


		radius += radiusDelta(stepRng);
		radius = std::clamp(radius, cave.radiusMin, cave.radiusMax);

	}

}

std::vector<CaveSeed> Chunk::BuildCavesFromSourceChunk(int scx, int scz) {
	std::vector<CaveSeed> caves;


	std::mt19937 rng(makeChunkSeed(gWorld->getWorldSeed() + kCaveSalt, scx, scz));

	std::uniform_int_distribution<int> chance(0, 99);
	if (chance(rng) >= 15) return caves;

	std::uniform_int_distribution<int> caveCountDist(1, 3);
	int caveCount = caveCountDist(rng);

	std::uniform_int_distribution<int> xDist(2, CHUNK_WIDTH - 3);
	std::uniform_int_distribution<int> zDist(2, CHUNK_WIDTH - 3);
	std::uniform_int_distribution<int> depthDist(12, 40);
	std::uniform_int_distribution<int> stepDist(40, 100);
	std::uniform_real_distribution<float> angleDist(0.0f, 6.283185f);
	std::uniform_real_distribution<float> pitchDist(-0.12f, 0.12f);
	std::uniform_real_distribution<float> radiusStartDist(1.6f, 2.8f);
	std::uniform_real_distribution<float> turnDist(0.12f, 0.35f);
	std::uniform_real_distribution<float> riseDist(0.04f, 0.12f);
	std::uniform_real_distribution<float> jitterDist(0.08f, 0.18f);
	std::uniform_int_distribution<int> roomChanceDist(3, 10);
	std::uniform_int_distribution<int> typeRoll(0, 99);

	for (int i = 0; i < caveCount; i++) {
		int lx = xDist(rng);
		int lz = zDist(rng);

		int wx = scx * CHUNK_WIDTH + lx;
		int wz = scz * CHUNK_WIDTH + lz;
		int surfaceY = GetSurfaceHeight(wx, wz);

		int minY = 6;
		int maxY = std::max(minY, surfaceY - 8);
		int wy = std::clamp(surfaceY - depthDist(rng), minY, maxY);

		CaveSeed cave{};

		
		int t = typeRoll(rng);

		if (t < 80) {
			cave.type = CaveType::MainTunnel;
		}
		else {
			cave.type = CaveType::Roomy;
		}
		cave.stepSeed = rng();

		cave.startX = static_cast<float>(wx);
		cave.startY = static_cast<float>(wy);
		cave.startZ = static_cast<float>(wz);

		cave.yaw = angleDist(rng);
		cave.pitch = pitchDist(rng);
		cave.steps = stepDist(rng) * 3;

		cave.radiusStart = radiusStartDist(rng);
		cave.radiusMin = 1.2f;
		cave.radiusMax = 3.8f;

		cave.turnStrength = turnDist(rng);
		cave.turnStrength *= 0.5f;

		cave.riseStrength = riseDist(rng);
		cave.radiusJitter = jitterDist(rng);

		cave.roomChancePercent = roomChanceDist(rng);

		caves.push_back(cave);
	}

	return caves;
}

void Chunk::GenerateCave(std::mt19937& rng) {
	
	std::uniform_int_distribution<int> xDist(2, CHUNK_WIDTH - 3);
	std::uniform_int_distribution<int> zDist(2, CHUNK_WIDTH - 3);
	std::uniform_int_distribution<int> depthDist(12, 40);
	std::uniform_int_distribution<int> stepCountDist(30, 90);
	std::uniform_real_distribution<float> turnDist(-0.35f, 0.35f);
	std::uniform_real_distribution<float> riseDist(-0.12f, 0.12f);
	std::uniform_real_distribution<float> radiusJitter(-0.15f, 0.15f);
	std::uniform_int_distribution<int> roomChance(0, 99);

	int startX = xDist(rng);
	int startZ = zDist(rng);

	int wx = cx * CHUNK_WIDTH + startX;
	int wz = cz * CHUNK_WIDTH + startZ;
	int surfaceY = GetSurfaceHeight(wx, wz);

	int minY = 6;
	int maxY = std::max(minY, surfaceY - 8);
	int startY = std::clamp(surfaceY - depthDist(rng), minY, maxY);

	float x = static_cast<float>(startX);
	float y = static_cast<float>(startY);
	float z = static_cast<float>(startZ);

	std::uniform_real_distribution<float> angleDist(0.0f, 6.28318f);
	float yaw = angleDist(rng);
	float pitch = riseDist(rng);

	float radius = 1.8f;
	int steps = stepCountDist(rng);

	for (int i = 0; i < steps; i++) {


		int carveRadius = std::max(1, static_cast<int>(std::round(radius)));
		CarveSphere((int)std::round(x), (int)std::round(y), (int)std::round(z), carveRadius);

		if (roomChance(rng) < 6) {
			CarveSphere((int)std::round(x), (int)std::round(y), (int)std::round(z), carveRadius + 2);
		}

		yaw += turnDist(rng);
		pitch += riseDist(rng) * 0.35f;
		pitch = std::clamp(pitch, -0.45f, 0.45f);

		float dx = std::cos(pitch) * std::cos(yaw);
		float dy = std::sin(pitch);
		float dz = std::cos(pitch) * std::sin(yaw);

		x += dx;
		y += dy;
		z += dz;

		int lx = (int)std::round(x);
		int lz = (int)std::round(z);

		lx = std::clamp(lx, 1, CHUNK_WIDTH - 2);
		lz = std::clamp(lz, 1, CHUNK_WIDTH - 2);
		x = (float)lx;
		z = (float)lz;

		int curWx = cx * CHUNK_WIDTH + lx;
		int curWz = cz * CHUNK_WIDTH + lz;
		int curSurfaceY = GetSurfaceHeight(curWx, curWz);

		int curMaxY = std::max(minY, curSurfaceY - 6);
		y = std::clamp(y, (float)minY, (float)curMaxY);

		radius += radiusJitter(rng);
		radius = std::clamp(radius, 1.3f, 3.2f);
	}
}
#pragma endregion ChunkGenerationFuncs

void Chunk::generate() {
	uint32_t chunkSeed = makeChunkSeed(gWorld->getWorldSeed(), cx, cz);
	std::mt19937 rng(chunkSeed);

	int ground = CHUNK_HEIGHT / 2;

	FillTerrain();
	GenerateTrees(rng);
	ApplyCaves();
	/*GenerateStoneBlobs(rng, ground);
	ScatterOre(rng, ground);
	GenerateOreVein(rng, ground);
	*/

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
					if (target == nullptr) return false;
					if (!target->isGenerated) return false;

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


	if (vertices.empty()) {
		vertexCount = 0;
		return; // データがないなら転送せずに終わる
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
	isEdited = false;

}


void Chunk::render() {

	if (vertexCount == 0) return;

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	glBindVertexArray(0);
}

int Chunk::GetSurfaceHeight(int wx, int wz) const {
	uint32_t seed = gWorld->getWorldSeed();

	float continent = FractalNoise2D(wx * 0.0010f, wz * 0.0010f, seed + 10);
	float hills = FractalNoise2D(wx * 0.0050f, wz * 0.0050f, seed + 20);
	float biome = FractalNoise2D(wx * 0.0007f, wz * 0.0007f, seed + 100);

	float mountainMask = FractalNoise2D(wx * 0.0010f, wz * 0.0010f, seed + 30);
	float mountainBase = FractalNoise2D(wx * 0.0014f, wz * 0.0014f, seed + 50);   // smooth にする
	float mountainShape = RidgedNoise2D(wx * 0.0035f, wz * 0.0035f, seed + 60);
	float mountainDetail = FractalNoise2D(wx * 0.0100f, wz * 0.0100f, seed + 70);

	float c = (continent - 0.5f) * 2.0f;
	float h = (hills - 0.5f) * 2.0f;

	float baseHeight = 62.0f + c * 10.0f;

	// plains/hills 側
	float lowlandHeight = baseHeight;
	if (biome < 0.25f) {
		lowlandHeight += h * 2.0f;
	}
	else {
		lowlandHeight += h * 8.0f;
	}

	// mountains 側
	float m = (mountainMask - 0.10f) / 0.90f;
	m = std::clamp(m, 0.0f, 1.0f);
	float mw = std::pow(m, 1.4f);

	float massif = std::pow(mountainBase, 1.15f) * 130.0f;
	float shape = std::pow(mountainShape, 2.0f) * 90.0f;
	float rough = (mountainDetail - 0.5f) * 16.0f;

	float mountainHeight = baseHeight + (massif + shape + rough) * mw;

	// hills -> mountains を滑らかに補間
	float t = (biome - 0.35f) / (0.60f - 0.35f);
	t = std::clamp(t, 0.0f, 1.0f);

	// smoothstep
	t = t * t * (3.0f - 2.0f * t);

	float height = lowlandHeight * (1.0f - t) + mountainHeight * t;

	int finalHeight = (int)std::floor(height);
	return std::clamp(finalHeight, 1, CHUNK_HEIGHT - 1);
}

