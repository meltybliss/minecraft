#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "game.h"
#include <fstream>
#include <sstream>
#include <string>
#include "Mat4.h"
#include "texture.h"
#include "ShaderInitUtils.h"



int main() {
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(800, 600, "MyWindow", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	Game game;
	

	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, 800, 600);
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

	

	unsigned int program = CreateShaderProgram("resources/shaders/basic.vert", "resources/shaders/basic.frag");

	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	GLuint blockTex = LoadTexture2D("resources/textures/atlas.png");

	glUseProgram(program);
	int texLoc = glGetUniformLocation(program, "uTexture");
	int viewLoc = glGetUniformLocation(program, "uView");
	int projLoc = glGetUniformLocation(program, "uProj");


	float lastTime = (float)glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		float curTime = (float)glfwGetTime();
		float deltaTime = curTime - lastTime;
		lastTime = curTime;

		glfwPollEvents();
		game.Tick(deltaTime);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, blockTex);


		glUniform1i(texLoc, 0);

		const Camera& cam = game.GetPlayer().GetCamera();

		Vec3 eye = cam.pos;
		Vec3 center = cam.pos + cam.forward;

		Mat4 view = LookAt(eye, center, cam.up);
		Mat4 proj = Perspective(70.0f, 800.0f / 600.0f, 0.1f, 1000.0f);

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.m);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj.m);

		game.Render(program);

		glfwSwapBuffers(window);
	}

	glDeleteProgram(program);

	glfwTerminate();
	return 0;

}