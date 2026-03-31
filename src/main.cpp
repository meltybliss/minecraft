#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "game.h"
#include <fstream>
#include <sstream>
#include <string>
#include "Mat4.h"



std::string ReadFile(const std::string& path) {
	std::ifstream file(path);
	if (!file) {
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();


	return buffer.str();

}

unsigned int CompileShader(unsigned int type, const std::string& source) {
	unsigned int shader = glCreateShader(type);
	const char* src = source.c_str();

	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	int success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);


	}

	return shader;

}


unsigned int CreateShaderProgram(const std::string& vertPath, const std::string& fragPath) {
	std::string vertSource = ReadFile(vertPath);
	std::string fragSource = ReadFile(fragPath);

	unsigned int vertexShader = CompileShader(GL_VERTEX_SHADER, vertSource);
	unsigned int fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragSource);

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	int success = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int main() {
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(800, 600, "MyWindow", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, 800, 600);
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

	Game game;

	unsigned int program = CreateShaderProgram("shaders/basic.vert", "shaders/basic.frag");

	


	float lastTime = (float)glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		float curTime = (float)glfwGetTime();
		float deltaTime = curTime - lastTime;
		lastTime = curTime;

		glfwPollEvents();

		//tick
		game.Tick(deltaTime);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//openGl draw
		glUseProgram(program);

		Vec3 eye = game.cam.pos;
		Vec3 center = game.cam.pos + game.cam.forward;
		
		Mat4 view = LookAt(eye, center, game.cam.up);
		Mat4 proj = Perspective(70.0f, 800.0f / 600.0f, 0.1f, 1000.0f);

		int viewLoc = glGetUniformLocation(program, "uView");
		int projLoc = glGetUniformLocation(program, "uProj");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.m);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj.m);


		game.Render();

		glfwSwapBuffers(window);

	}

	glDeleteProgram(program);

	glfwTerminate();
	return 0;

}