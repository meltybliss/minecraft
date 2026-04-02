#include "Player.h"
#include "world.h"


void Player::Tick(float dt) {
    GLFWwindow* window = glfwGetCurrentContext();
    UpdatePlrMovement(window, dt);
    UpdateMouse();
}

void Player::UpdatePlrMovement(GLFWwindow* window, float dt) {

	Vec3 forward = cam.forward;
	forward.y = 0.0f;
	if (Length(forward) > 0.0f) forward = Normalize(forward);

    Vec3 right = Normalize(Cross(forward, Vec3{ 0, 1, 0 }));
    Vec3 move = { 0, 0, 0 };

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move = move + forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move = move - forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move = move - right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move = move + right;

    if (Length(move) > 0.0f) move = Normalize(move);

    pos += move * walkSpeed * dt;
    this->UpdateCamera();
}


void Player::UpdateMouse() {
    Ray ray = cam.GetRay(0.5f, 0.5f);
    lastHit = gWorld->TraceRay(ray, 5.0f);

    static bool prevLeftDown = false;
    static bool prevRightDown = false;
    bool leftDown = glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool rightDown = glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    if (leftDown && !prevLeftDown) {
        if (lastHit.isHit) {
            gWorld->SetBlockGlobal((int)lastHit.hitPos.x,
                (int)lastHit.hitPos.y,
                (int)lastHit.hitPos.z,
                (unsigned int)BlockType::AIR);
        }
    }

    if (rightDown && !prevRightDown) {
        if (lastHit.isHit) {

            int nx = static_cast<int>(std::floor(lastHit.hitPos.x + lastHit.normal.x));
            int ny = static_cast<int>(std::floor(lastHit.hitPos.y + lastHit.normal.y));
            int nz = static_cast<int>(std::floor(lastHit.hitPos.z + lastHit.normal.z));

            gWorld->SetBlockGlobal(nx, ny, nz, (unsigned int)selectedBlock);
        }
    }

    prevLeftDown = leftDown;
    prevRightDown = rightDown;

}