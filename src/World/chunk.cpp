#include "World/chunk.h"
#include "World/world.h"


Chunk::Chunk() {

	
}

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


void Chunk::renderSolid(GLuint program) {
	if (vertexCount <= 0) return;

	GLint alphaLoc = glGetUniformLocation(program, "uAlpha");

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glUniform1f(alphaLoc, 1.0f);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	glBindVertexArray(0);
}

void Chunk::renderWater(GLuint program) {
	if (waterVertexCount <= 0) return;

	GLint alphaLoc = glGetUniformLocation(program, "uAlpha");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glUniform1f(alphaLoc, 0.5f);

	glBindVertexArray(waterVAO);
	glDrawArrays(GL_TRIANGLES, 0, waterVertexCount);
	glBindVertexArray(0);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}