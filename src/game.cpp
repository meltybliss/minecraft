#include "game.h"

Game* gGame = nullptr;

Game::Game() {
	gGame = this;
	
	glfwSetCursorPosCallback(glfwGetCurrentContext(), MouseCallback);
	glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);


}


void Game::Tick(float deltaTime) {
	
	world.Tick();
	plr.Tick(deltaTime);
}

void Game::Render() {
	world.render();

}