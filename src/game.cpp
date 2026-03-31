#include "game.h"

Game::Game() {
	gGame = this;
	cam.UpdateVectors();

	glfwSetWindowUserPointer(glfwGetCurrentContext(), &cam);
	glfwSetCursorPosCallback(glfwGetCurrentContext(), MouseCallback);
	glfwGetInputMode(glfwGetCurrentContext(), GLFW_CURSOR);
}

void Game::Tick(float deltaTime) {
	UpdateCameraMovement(glfwGetCurrentContext(), cam, deltaTime);
	world.Tick();
}

void Game::Render() {
	world.render();
}