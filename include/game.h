#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "world.h"
#include "camera.h"
#include <iostream>

class Game {
public:
	Game();

	void Tick(float deltaTime);
	void Render();
	Camera cam;
	
private:
	World world;
	HitResult lastHit;
};

extern Game* gGame;