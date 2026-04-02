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

	if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

		Ray ray = cam.GetRay(0.5f, 0.5f);
		world.SetBlockByRay(ray, 0, 5.0f);
		
	};

	world.Tick();
}

void Game::Render() {
	world.render();
}