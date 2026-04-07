#include "World/GetSurfaceHeight.h"
#include "World/world.h"

int GetSurfaceHeight(int wx, int wz)
{
    uint32_t seed = gWorld->getWorldSeed();

    float continent = FractalNoise2D(wx * 0.0010f, wz * 0.0010f, seed + 10);
    float hills = FractalNoise2D(wx * 0.0050f, wz * 0.0050f, seed + 20);
    float biome = FractalNoise2D(wx * 0.0007f, wz * 0.0007f, seed + 100);

    float mountainMask = FractalNoise2D(wx * 0.0010f, wz * 0.0010f, seed + 30);
    float mountainBase = FractalNoise2D(wx * 0.0014f, wz * 0.0014f, seed + 50);
    float mountainShape = RidgedNoise2D(wx * 0.0035f, wz * 0.0035f, seed + 60);
    float mountainDetail = FractalNoise2D(wx * 0.0100f, wz * 0.0100f, seed + 70);

    float c = (continent - 0.5f) * 2.0f;
    float h = (hills - 0.5f) * 2.0f;

    float baseHeight = 62.0f + c * 10.0f;

    float lowlandHeight = baseHeight;
    if (biome < 0.25f) {
        lowlandHeight += h * 2.0f;
    }
    else {
        lowlandHeight += h * 8.0f;
    }

    float m = (mountainMask - 0.10f) / 0.90f;
    m = std::clamp(m, 0.0f, 1.0f);
    float mw = std::pow(m, 1.4f);

    float massif = std::pow(mountainBase, 1.15f) * 130.0f;
    float shape = std::pow(mountainShape, 2.0f) * 90.0f;
    float rough = (mountainDetail - 0.5f) * 16.0f;

    float mountainHeight = baseHeight + (massif + shape + rough) * mw;

    float t = (biome - 0.35f) / (0.60f - 0.35f);
    t = std::clamp(t, 0.0f, 1.0f);
    t = t * t * (3.0f - 2.0f * t);

    float height = lowlandHeight * (1.0f - t) + mountainHeight * t;

    // ’ácontinent—Ìˆæ‚ð’Ç‰Á‚Å’¾‚ß‚é
    if (c < -0.15f) height -= 6.0f;
    if (c < -0.35f) height -= 8.0f;
    if (c < -0.55f) height -= 12.0f;

    int finalHeight = (int)std::floor(height);
    return std::clamp(finalHeight, 1, Chunk::CHUNK_HEIGHT - 1);
}