#include "CaveGenerator.h"
#include "world.h"
#include "MathUtils.h"

namespace {
	
	constexpr int kCaveSalt = 9999u;

}

void CaveGenerator::ApplyCaves(Chunk* c) {
	static int R = 1;

	for (int scz = c->cz - R; scz <= c->cz + R; scz++) {
		for (int scx = c->cx - R; scx <= c->cx + R; scx++) {
			ApplyCavesFromSourceChunk(c, scx, scz);
		}
	}

}

void CaveGenerator::ApplyCavesFromSourceChunk(Chunk* c, int scx, int scz) {

	uint64_t key = Chunk::GetChunkKey(scx, scz);

	auto it = c->cavesCashe.find(key);
	if (it == c->cavesCashe.end()) {
		auto caves = BuildCavesFromSourceChunk(c, scx, scz);
		auto [insertedIt, _] = c->cavesCashe.emplace(key, std::move(caves));
		it = insertedIt;
	}


	for (const auto& cave : it->second) {
		ApplySingleCave(c, cave, 0);
	}


}



void CaveGenerator::ApplySingleCave(Chunk* c, const CaveSeed& cave, int depth) {
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

	int chunkMinX = c->cx * Chunk::CHUNK_WIDTH;
	int chunkMaxX = chunkMinX + Chunk::CHUNK_WIDTH - 1;
	int chunkMinZ = c->cz * Chunk::CHUNK_WIDTH;
	int chunkMaxZ = chunkMinZ + Chunk::CHUNK_WIDTH - 1;

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

			ApplySingleCave(c, branch, depth + 1);
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

			CarveEllipsoid(c, localX, localY, localZ, rx, ry, rz);

			if (roomRoll(stepRng) < cave.roomChancePercent) {
				CarveEllipsoid(c, localX, localY, localZ, rx + 2, ry + 1, rz + 2);
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


std::vector<CaveSeed> CaveGenerator::BuildCavesFromSourceChunk(Chunk* c, int scx, int scz) {
	std::vector<CaveSeed> caves;


	std::mt19937 rng(Chunk::makeChunkSeed(gWorld->getWorldSeed() + kCaveSalt, scx, scz));

	std::uniform_int_distribution<int> chance(0, 99);
	if (chance(rng) >= 15) return caves;

	std::uniform_int_distribution<int> caveCountDist(1, 3);
	int caveCount = caveCountDist(rng);

	std::uniform_int_distribution<int> xDist(2, Chunk::CHUNK_WIDTH - 3);
	std::uniform_int_distribution<int> zDist(2, Chunk::CHUNK_WIDTH - 3);
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

		int wx = scx * Chunk::CHUNK_WIDTH + lx;
		int wz = scz * Chunk::CHUNK_WIDTH + lz;
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
