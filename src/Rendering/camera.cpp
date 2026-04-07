#include "Rendering/camera.h"
#include "Core/game.h"

Ray Camera::GetRay(float u, float v) const {
    return { pos, Normalize(lowerLeftCorner + (horizontal * u) + (vertical * v) - pos) };
}

Mat4 Camera::GetViewMatrix() const {
    return LookAt(pos, pos + forward, up);
}

Mat4 Camera::GetProjectionMatrix() const {
    return Perspective(fovDeg, aspect, nearZ, farZ);
}

void Camera::UpdateVectors() {
    const float radYaw = yaw * 3.14159265f / 180.0f;
    const float radPitch = pitch * 3.14159265f / 180.0f;

    Vec3 f;
    f.x = std::cos(radYaw) * std::cos(radPitch);
    f.y = std::sin(radPitch);
    f.z = std::sin(radYaw) * std::cos(radPitch);
    forward = Normalize(f);

    Vec3 worldUp{ 0.0f, 1.0f, 0.0f };
    right = Normalize(Cross(forward, worldUp));
    up = Normalize(Cross(right, forward));

    float fovRad = fovDeg * 3.14159265f / 180.0f;
    float focal_length = 1.0f;
    float viewport_height = 2.0f * std::tan(fovRad * 0.5f) * focal_length;
    float viewport_width = aspect * viewport_height;

    horizontal = right * viewport_width;
    vertical = up * viewport_height;
    lowerLeftCorner = pos + (forward * focal_length) - (horizontal * 0.5f) - (vertical * 0.5f);
}

void UpdateCameraMovement(GLFWwindow* window, Camera& cam, float deltaTime) {
    float speed = 10.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cam.pos += cam.forward * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cam.pos -= cam.forward * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cam.pos -= cam.right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cam.pos += cam.right * speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) cam.pos.y += speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) cam.pos.y -= speed;
}

void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true;
    static double lastX = 400.0;
    static double lastY = 300.0;

    if (!gGame) return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - lastX);
    float yoffset = static_cast<float>(lastY - ypos);

    lastX = xpos;
    lastY = ypos;

    gGame->GetPlayer().OnMouseMove(xoffset, yoffset);
}