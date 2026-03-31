#include "GLFW/glfw3.h"
#include "glad/glad.h"

int main() {
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(800, 600, "MyWindow", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		//tick


		glClear(GL_COLOR_BUFFER_BIT);

		//openGl draw

		glfwSwapBuffers(window);

	}

	glfwTerminate();
	return 0;

}