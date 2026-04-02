#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "world.h"
#include "camera.h"
#include <iostream>
#include "player.h"
class Game {
public:
	Game();

	void Tick(float deltaTime);
	void Render();
	
	Player& GetPlayer() { return plr; }
	const Player& GetPlayer() const { return plr; }

private:
	World world;
	Player plr;
};

extern Game* gGame;