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
	static bool prevRightDown = false;
	bool leftDown = glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	bool rightDown = glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

	if (leftDown && !prevLeftDown) {
		if (lastHit.isHit) {
			world.SetBlockGlobal((int)lastHit.hitPos.x,
				(int)lastHit.hitPos.y,
				(int)lastHit.hitPos.z,
				(unsigned int)BlockType::AIR);
		}
	}

	if (rightDown && !prevRightDown) {
		if (lastHit.isHit) {

			int nx = static_cast<int>(std::floor(lastHit.hitPos.x + lastHit.normal.x));
			int ny = static_cast<int>(std::floor(lastHit.hitPos.y + lastHit.normal.y));
			int nz = static_cast<int>(std::floor(lastHit.hitPos.z + lastHit.normal.z));

			world.SetBlockGlobal(nx, ny, nz, (unsigned int)BlockType::Dirt);
		}
	}

	prevLeftDown = leftDown;
	prevRightDown = rightDown;

	world.Tick();
}

void Game::Render() {
	world.render();

}