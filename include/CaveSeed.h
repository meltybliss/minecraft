#pragma once
struct CaveSeed {
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

    uint32_t stepSeed;
};