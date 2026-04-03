#pragma once
#include <stdint.h>
#include <cmath>

float FractalNoise2D(float x, float z, uint32_t seed);
float PerlinNoise2D(float x, float z, uint32_t seed);