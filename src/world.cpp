#include "world.h"
#include "game.h"

World* gWorld = nullptr;


static int32_t FloorDiv(int v, int b) {
	return static_cast<int32_t>(std::floor(static_cast<float>(v) / b));
}

World::World() {
	gWorld = this;
}

void World::Tick() {

	Vec3 pos = gGame->cam.pos;
	float chunkWorldSize = static_cast<float>(Chunk::CHUNK_WIDTH * blockSize);

	int32_t curCx = static_cast<int32_t>(std::floor(pos.x / chunkWorldSize));
	int32_t curCz = static_cast<int32_t>(std::floor(pos.z / chunkWorldSize));


	for (int32_t x = curCx - RENDER_DISTANCE; x <= curCx + RENDER_DISTANCE; x++) {
		for (int32_t z = curCz - RENDER_DISTANCE; z <= curCz + RENDER_DISTANCE; z++) {

			if (std::abs(x) >= MaxCX || std::abs(z) >= MaxCZ) continue;

			uint64_t key = GetChunkKey(x, z);
			if (Chunks.find(key) == Chunks.end()) {


				auto c = std::make_unique<Chunk>();
				c->cx = x;
				c->cz = z;

				c->generate();
				Chunks[key] = std::move(c);

				MarkChunkDirty(x, z);
				MarkChunkDirty(x + 1, z);
				MarkChunkDirty(x - 1, z);
				MarkChunkDirty(x, z + 1);
				MarkChunkDirty(x, z - 1);
				
			}
		}
	}

	std::erase_if(Chunks, [&](const auto& item) {
		const auto& c = item.second;
		if (c == nullptr) return false;

		int32_t dx = std::abs(c->cx - curCx);
		int32_t dz = std::abs(c->cz - curCz);

		return (dx >= UNLOAD_DISTANCE || dz >= UNLOAD_DISTANCE);

	});
}


void World::render() {
	for (auto& item : Chunks) {
		auto& c = item.second;
		
		if (c->isDirty) {

			c->buildMesh();
		}

		c->render();
	}
}


unsigned int World::GetBlockGlobal(int wx, int wy, int wz) {
	int32_t cx = FloorDiv(wx, Chunk::CHUNK_WIDTH * blockSize);
	int32_t cz = FloorDiv(wz, Chunk::CHUNK_WIDTH * blockSize);

	uint64_t key = GetChunkKey(cx, cz);
	if (Chunks.find(key) == Chunks.end()) return 0;

	Chunk* c = Chunks[key].get();
	int lx = wx - cx * Chunk::CHUNK_WIDTH;
	int ly = wy;
	int lz = wz - cz * Chunk::CHUNK_WIDTH;

	return c->Get(lx, ly, lz);
}

Chunk* World::GetChunkPtr(int cx, int cz) {
	uint64_t key = GetChunkKey(cx, cz);
	if (Chunks.find(key) == Chunks.end()) return nullptr;
	return Chunks[key].get();
}


void World::MarkChunkDirty(int32_t cx, int32_t cz) {
	uint64_t key = GetChunkKey(cx, cz);

	auto it = Chunks.find(key);
	if (it == Chunks.end() || !it->second) return;
	
	it->second->isDirty = true;
}

bool World::SetBlockGlobal(int wx, int wy, int wz, unsigned int block) {
	int32_t cx = FloorDiv(wx, Chunk::CHUNK_WIDTH * blockSize);
	int32_t cz = FloorDiv(wz, Chunk::CHUNK_WIDTH * blockSize);

	uint64_t key = GetChunkKey(cx, cz);
	if (Chunks.find(key) == Chunks.end()) return false;

	Chunk* c = Chunks[key].get();
	int lx = wx - cx * Chunk::CHUNK_WIDTH;
	int ly = wy;
	int lz = wz - cz * Chunk::CHUNK_WIDTH;

	MarkChunkDirty(cx, cz);

	return c->Set(lx, ly, lz, block);
}


bool World::SetBlockByRay(Ray& ray, unsigned int block, float maxDist) {
	HitResult hit = TraceRay(ray, maxDist);
	if (!hit.isHit) return false;

	return this->SetBlockGlobal(hit.hitPos.x, hit.hitPos.y, hit.hitPos.z, block);
	
}


HitResult World::TraceRay(Ray& ray, float maxDist) {
	//DDA algo

	HitResult hit = { false, {0,0,0}, {0,0,0}, 0.0f };

	int x = std::floor(ray.origin.x);
	int y = std::floor(ray.origin.y);
	int z = std::floor(ray.origin.z);

	Vec3 deltaDist = {
		std::abs(1.0f / ray.dir.x),
		std::abs(1.0f / ray.dir.y),
		std::abs(1.0f / ray.dir.z)
	};

	Vec3 sideDist;
	int stepX, stepY, stepZ;

	if (ray.dir.x < 0) {
		stepX = -1;
		sideDist.x = (ray.origin.x - x) * deltaDist.x;
	}
	else {
		stepX = 1;
		sideDist.x = (x + 1.0f - ray.origin.x) * deltaDist.x;
	}

	if (ray.dir.y < 0) {
		stepY = -1;
		sideDist.y = (ray.origin.y - y) * deltaDist.y;
	}
	else {
		stepY = 1;
		sideDist.y = (y + 1.0f - ray.origin.y) * deltaDist.y;
	}

	if (ray.dir.z < 0) {
		stepZ = -1;
		sideDist.z = (ray.origin.z - z) * deltaDist.z;
	}
	else {
		stepZ = 1;
		sideDist.z = (z + 1.0f - ray.origin.z) * deltaDist.z;
	}


	//DDA main loop
	float t = 0;
	while (t < maxDist) {
		if (sideDist.x < sideDist.y && sideDist.x < sideDist.z) {
			t = sideDist.x;
			sideDist.x += deltaDist.x;
			x += stepX;
			hit.normal = { -(float)stepX, 0, 0 };
		}
		else if (sideDist.y < sideDist.z) {
			t = sideDist.y;
			sideDist.y += deltaDist.y;
			y += stepY;
			hit.normal = { 0, -(float)stepY, 0 };
		}
		else {
			t = sideDist.z;
			sideDist.z += deltaDist.z;
			z += stepZ;
			hit.normal = { 0, 0, -(float)stepZ };
		}


		if (GetBlockGlobal(x, y, z) != (unsigned int)BlockType::AIR) {
			hit.isHit = true;
			hit.hitPos = { (float)x, (float)y, (float)z };
			hit.dist = t;
			return hit;
		}


	}

	return hit;

}


void World::InitRenderer() {
	selectionShaderProgram = CreateShaderProgram("shaders/wireframe.vert", "shaders/wireframe.frag");
	float vertices[] = {
		0,0,0, 1,0,0,  1,0,0, 1,0,1,  1,0,1, 0,0,1,  0,0,1, 0,0,0,
		0,1,0, 1,1,0,  1,1,0, 1,1,1,  1,1,1, 0,1,1,  0,1,1, 0,1,0,
		0,0,0, 0,1,0,  1,0,0, 1,1,0,  1,0,1, 1,1,1,  0,0,1, 0,1,1
	};
	glGenVertexArrays(1, &highlightVAO);
	glGenBuffers(1, &highlightVBO);
	glBindVertexArray(highlightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, highlightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}


void World::RenderHighlight(const HitResult& hit) {
	if (!hit.isHit) return;

	glUseProgram(selectionShaderProgram);

	float s = 1.005f;
	float o = (s - 1.0f) * 0.5f;

	// hit.hitPos が整数 {x, y, z} であることを前提にする
	Mat4 translation = Mat4::Translate(Vec3{ hit.hitPos.x - o, hit.hitPos.y - o, hit.hitPos.z - o });
	Mat4 scaling = Mat4::Scale(Vec3{ s, s, s });
	Mat4 model = translation * scaling;

	// カメラクラスから取得（mainのPerspective計算と数値を合わせる）
	Mat4 view = gGame->cam.GetViewMatrix();
	Mat4 projection = gGame->cam.GetProjectionMatrix();

	glUniformMatrix4fv(glGetUniformLocation(selectionShaderProgram, "model"), 1, GL_FALSE, model.m);
	glUniformMatrix4fv(glGetUniformLocation(selectionShaderProgram, "view"), 1, GL_FALSE, view.m);
	glUniformMatrix4fv(glGetUniformLocation(selectionShaderProgram, "projection"), 1, GL_FALSE, projection.m);

	// color を確実に白にする
	glUniform3f(glGetUniformLocation(selectionShaderProgram, "color"), 1.0f, 1.0f, 1.0f);

	glBindVertexArray(highlightVAO);
	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, 24);
	glBindVertexArray(0);
	glUseProgram(0); // 念のため解除
}