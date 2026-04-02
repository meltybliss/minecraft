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

	static bool prevLeftDown = false;
	bool leftDown = glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

	if (leftDown && !prevLeftDown) {
		if (lastHit.isHit) {
			world.SetBlockGlobal((int)lastHit.hitPos.x,
				(int)lastHit.hitPos.y,
				(int)lastHit.hitPos.z,
				(unsigned int)BlockType::AIR);
		}
	}

	prevLeftDown = leftDown;

	world.Tick();
}

void Game::Render() {
	world.render();

}