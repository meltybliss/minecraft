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

    if (grounded && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) { vel.y = jmpPower; grounded = false; }

    if (Length(move) > 0.0f) move = Normalize(move);

    vel.x = move.x * walkSpeed;
    vel.z = move.z * walkSpeed;
    vel.y += gravity * dt;

    pos.x += vel.x * dt;
    if (IntersectsSolidBlock(GetAABBAt(pos))) {
        pos.x -= vel.x * dt;
        vel.x = 0.0f;
    }

    pos.z += vel.z * dt;
    if (IntersectsSolidBlock(GetAABBAt(pos))) {
        pos.z -= vel.z * dt;
        vel.z = 0.0f;
    }

    pos.y += vel.y * dt;
    if (IntersectsSolidBlock(GetAABBAt(pos))) {
        pos.y -= vel.y * dt;
        if (vel.y < 0.0f) {
            grounded = true;
        }

        vel.y = 0.0f;
    }


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

            if (IsPlaceable(GetAABBAt(pos), Vec3{(float)nx, (float)ny, (float)nz})) {

                gWorld->SetBlockGlobal(nx, ny, nz, (unsigned int)selectedBlock);
            }
        }
    }

    prevLeftDown = leftDown;
    prevRightDown = rightDown;

}


AABB Player::GetAABBAt(const Vec3& p) const {
    return {
        Vec3{p.x - radius, p.y, p.z - radius},
        Vec3{p.x + radius, p.y + height, p.z + radius}
    };
}


bool Player::IntersectsSolidBlock(const AABB& box) {
    int minX = (int)std::floor(box.min.x);
    int maxX = (int)std::floor(box.max.x);
    int minY = (int)std::floor(box.min.y);
    int maxY = (int)std::floor(box.max.y);
    int minZ = (int)std::floor(box.min.z);
    int maxZ = (int)std::floor(box.max.z);

    for (int y = minY; y <= maxY; y++) {
        for (int z = minZ; z <= maxZ; z++) {
            for (int x = minX; x <= maxX; x++) {
                if (gWorld->GetBlockGlobal(x, y, z) != 0) {
                    return true;
                }
            }
        }
    }

    return false;
}


bool Player::CanPlaceBlockAt(Vec3 blockPos) const {
    const AABB& box = GetAABBAt(pos);

    int minX = (int)std::floor(box.min.x);
    int maxX = (int)std::floor(box.max.x);
    int minY = (int)std::floor(box.min.y);
    int maxY = (int)std::floor(box.max.y);
    int minZ = (int)std::floor(box.min.z);
    int maxZ = (int)std::floor(box.max.z);

    int bx = (int)std::floor(blockPos.x);
    int by = (int)std::floor(blockPos.y);
    int bz = (int)std::floor(blockPos.z);

    return (!(bx >= minX && bx <= maxX &&
        by >= minY && by <= maxY &&
        bz >= minZ && bz <= maxZ));

}