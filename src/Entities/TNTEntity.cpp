#include "Entities/TNTEntity.h"
#include "ExplosionService.h"
#include <cmath>
#include "block.h"
#include "Mat4.h"


TNTEntity::TNTEntity(const Vec3& startPos, float startTimer)
    : Entity(startPos), timer(startTimer) {

    BlockRenderUtils::AppendBlockCube(
        verts, 0.0f, 0.0f, 0.0f,
        (unsigned int)BlockType::TNT
    );

    vertexCount = static_cast<GLsizei>(verts.size() / 5);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        verts.size() * sizeof(float),
        verts.data(),
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


TNTEntity::~TNTEntity() {
    if (vbo != 0) glDeleteBuffers(1, &vbo);
    if (vao != 0) glDeleteVertexArrays(1, &vao);

}

void TNTEntity::Tick(float dt) {
	timer -= dt;

	if (timer <= 0.0f) {

		ExplosionService::Explode(static_cast<int>(std::floor(pos.x)), static_cast<int>(std::floor(pos.y)),
			static_cast<int>(std::floor(pos.z)), 5);

		isDead = true;
	}

}

void TNTEntity::Render(GLuint program) {
    float flash = shouldFlash() ? 0.5f : 0.0f;

    glUniform1f(glGetUniformLocation(program, "uFlash"), flash);

    Mat4 model = Mat4::Translate(Vec3{ pos.x, pos.y, pos.z });
    glUniformMatrix4fv(glGetUniformLocation(program, "uModel"), 1, GL_FALSE, model.m);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}

bool TNTEntity::shouldFlash() const {

	int phase = static_cast<int>(timer * 5);
	return (phase % 2) == 0;
}