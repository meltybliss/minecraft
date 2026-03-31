#include "camera.h"

void UpdateCameraMovement(GLFWwindow* window, Camera& cam, float deltaTime) {
	float speed = 10.0f * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cam.pos += cam.forward * speed;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cam.pos -= cam.forward * speed;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cam.pos -= cam.right * speed;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cam.pos += cam.right * speed;
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		cam.pos.y += speed;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		cam.pos.y -= speed;
	}
}

void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
	static bool firstMouse = true;
	static double lastX = 400.0;
	static double lastY = 300.0;

	Camera* cam = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));
	if (!cam) return;

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = static_cast<float>(xpos - lastX);
	float yoffset = static_cast<float>(lastY - ypos);

	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	cam->yaw += xoffset;
	cam->pitch += yoffset;

	if (cam->pitch > 89.0f) cam->pitch = 89.0f;
	if (cam->pitch < -89.0f) cam->pitch = -89.0f;

	cam->UpdateVectors();
}
