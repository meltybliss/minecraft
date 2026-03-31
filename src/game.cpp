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
	world.Tick();
}

void Game::Render() {
	world.render();
}