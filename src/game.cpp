#include "game.h"

Game* gGame = nullptr;

Game::Game() {
	gGame = this;
	cam.UpdateVectors();

	

	glfwSetWindowUserPointer(glfwGetCurrentContext(), &cam);
	glfwSetCursorPosCallback(glfwGetCurrentContext(), MouseCallback);
	glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}


void Game::Tick(float deltaTime) {
	UpdateCameraMovement(glfwGetCurrentContext(), cam, deltaTime);

	Ray ray = cam.GetRay(0.5f, 0.5f);
	lastHit = world.TraceRay(ray, 5.0f);

	if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (lastHit.isHit) {
            // 当たっている座標のブロックを AIR(0) にする
            world.SetBlockGlobal((int)lastHit.hitPos.x, (int)lastHit.hitPos.y, (int)lastHit.hitPos.z, 0);
        }
    }

	world.Tick();
}

void Game::Render() {
	world.render();

	world.RenderHighlight(lastHit);
}