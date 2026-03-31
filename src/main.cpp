#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "game.h"
#include <fstream>
#include <sstream>
#include <string>




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

	glViewport(0, 0, 800, 600);
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

	Game game;

	unsigned int program = CreateShaderProgram("shaders/basic.vert", "shaders/basic.frag");

	float vertices[] = {
	 0.0f,  0.5f,
	-0.5f, -0.5f,
	 0.5f, -0.5f
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// location = 0 ‚Ì aPos ‚ÉA2ŒÂ‚¸‚Â float ‚ð“n‚·
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	float lastTime = (float)glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		float curTime = (float)glfwGetTime();
		float deltaTime = curTime - lastTime;
		lastTime = curTime;

		glfwPollEvents();

		//tick
		game.Tick(deltaTime);

		glClear(GL_COLOR_BUFFER_BIT);

		//openGl draw
		glUseProgram(program);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);

	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(program);

	glfwTerminate();
	return 0;

}