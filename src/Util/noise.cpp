#include "Util/noise.h"


static float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static void GetGradient(int ix, int iz, uint32_t seed, float& gx, float& gz) {
    uint32_t h = seed;
    h ^= static_cast<uint32_t>(ix) * 374761393u;
    h ^= static_cast<uint32_t>(iz) * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;

    float angle = (float)(h % 360) * (3.14159265f / 180.0f);
    gx = std::cos(angle);
    gz = std::sin(angle);
}

// 2. 内積を計算する補助関数
static float DotGridGradient(int ix, int iz, float x, float z, uint32_t seed) {
    float gx, gz;
    GetGradient(ix, iz, seed, gx, gz);
    float dx = x - (float)ix;
    float dz = z - (float)iz;
    return (dx * gx + dz * gz);
}

float PerlinNoise2D(float x, float z, uint32_t seed) {
    int x0 = (int)std::floor(x);
    int z0 = (int)std::floor(z);
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    float tx = x - (float)x0;
    float tz = z - (float)z0;

    // 5次式 (Quintic) の方がより滑らかになります (6t^5 - 15t^4 + 10t^3)
    auto Fade = [](float t) { return t * t * t * (t * (t * 6 - 15) + 10); };
    float sx = Fade(tx);
    float sz = Fade(tz);

    // 4つの角の影響度を計算
    float n00 = DotGridGradient(x0, z0, x, z, seed);
    float n10 = DotGridGradient(x1, z0, x, z, seed);
    float n01 = DotGridGradient(x0, z1, x, z, seed);
    float n11 = DotGridGradient(x1, z1, x, z, seed);

    // 線形補間 (Lerp)
    float ix0 = Lerp(n00, n10, sx);
    float ix1 = Lerp(n01, n11, sx);

    float value = Lerp(ix0, ix1, sz);

    // Perlinノイズの結果は -0.7 ~ 0.7 くらいになるので、0.0 ~ 1.0 に補正
    return value * 1.5f;
}

float FractalNoise2D(float x, float z, uint32_t seed) {
    float total = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < 4; i++) {
        // ValueNoise2D から PerlinNoise2D に変更！
        total += PerlinNoise2D(x * frequency, z * frequency, seed + i * 1013) * amplitude;

        maxValue += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    
    return (total / maxValue) * 0.5f + 0.5f;
}

float RidgedNoise2D(float x, float z, uint32_t seed) {
    float n = FractalNoise2D(x, z, seed); // 0..1
    n = 1.0f - std::abs(n * 2.0f - 1.0f);
    n = std::pow(n, 1.8f);
    return n;
}