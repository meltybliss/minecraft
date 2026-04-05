#include "game.h"

Game* gGame = nullptr;

Game::Game() {
	gGame = this;
	
	glfwSetCursorPosCallback(glfwGetCurrentContext(), MouseCallback);
	glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);


}


void Game::Tick(float deltaTime) {
	double curTime = glfwGetTime();
	nbFrames++;

	world.Tick();
	plr.Tick(deltaTime);

	if (curTime - lastTime >= 1.0) {
		std::string title = "MyMinecraft - " + std::to_string(nbFrames) + " FPS";
		glfwSetWindowTitle(glfwGetCurrentContext(), title.c_str());
		nbFrames = 0;
		lastTime = curTime;
	}
}

void Game::Render() {
	world.render();

}