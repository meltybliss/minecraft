#pragma once
#include <cmath>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Vec3.h"
#include "Ray.h"
#include "Mat4.h"

struct Camera {
    Vec3 pos{ 0.0f, 100.0f, 0.0f };

    float yaw = -90.0f;
    float pitch = 0.0f;

    Vec3 forward{ 0.0f, 0.0f, -1.0f };
    Vec3 right{ 1.0f, 0.0f, 0.0f };
    Vec3 up{ 0.0f, 1.0f, 0.0f };

    float fovDeg = 45.0f;
    float aspect = 16.0f / 9.0f;
    float nearZ = 0.1f;
    float farZ = 1000.0f;

    Vec3 lowerLeftCorner;
    Vec3 horizontal;
    Vec3 vertical;

    Ray GetRay(float u, float v) const;
    Mat4 GetViewMatrix() const;
    Mat4 GetProjectionMatrix() const;
    void UpdateVectors();
};

void UpdateCameraMovement(GLFWwindow* window, Camera& cam, float deltaTime);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);