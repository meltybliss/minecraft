#pragma once
#include <stdint.h>

enum class CaveType {
    MainTunnel,
    ThinBranch,
    Roomy
};

struct CaveSeed {
    uint32_t stepSeed;
    

    CaveType type;

    float startX;
    float startY;
    float startZ;

    float yaw;
    float pitch;
    int steps;

    float radiusStart;
    float radiusMin;
    float radiusMax;

    float turnStrength;
    float riseStrength;
    float radiusJitter;

    int roomChancePercent;
    int branchChancePercent;

};