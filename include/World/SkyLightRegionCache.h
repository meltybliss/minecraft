#pragma once
#include "chunk.h"
#include "Util/MathUtils.h"

struct SkylightRegionCache {
    static constexpr int REGION_CHUNKS = 3;
    Chunk* chunks[REGION_CHUNKS][REGION_CHUNKS] = {};

    int baseCx = 0;
    int baseCz = 0;

    Chunk* GetChunkByBlock(int bx, int bz) const {
        int cx = FloorDiv(bx, Chunk::CHUNK_WIDTH);
        int cz = FloorDiv(bz, Chunk::CHUNK_WIDTH);

        int rx = cx - baseCx;
        int rz = cz - baseCz;

        if (rx < 0 || rx >= REGION_CHUNKS) return nullptr;
        if (rz < 0 || rz >= REGION_CHUNKS) return nullptr;

        return chunks[rx][rz];
    }

    Block* GetBlockPtr(int bx, int by, int bz) const {
        if (by < 0 || by >= Chunk::CHUNK_HEIGHT) return nullptr;

        Chunk* c = GetChunkByBlock(bx, bz);
        if (!c) return nullptr;

        int cx = FloorDiv(bx, Chunk::CHUNK_WIDTH);
        int cz = FloorDiv(bz, Chunk::CHUNK_WIDTH);

        int lx = bx - cx * Chunk::CHUNK_WIDTH;
        int lz = bz - cz * Chunk::CHUNK_WIDTH;

        if (lx < 0 || lx >= Chunk::CHUNK_WIDTH) return nullptr;
        if (lz < 0 || lz >= Chunk::CHUNK_WIDTH) return nullptr;

        int idx = Chunk::Index(lx, by, lz);
        return &c->blocks[idx];
    }

    unsigned int GetBlockId(int bx, int by, int bz) const {
        Block* b = GetBlockPtr(bx, by, bz);
        if (!b) return (unsigned int)BlockType::Stone; // ŠO‚ÍŽÕ•Áˆµ‚¢
        return b->type;
    }

    uint8_t GetSky(int bx, int by, int bz) const {
        Block* b = GetBlockPtr(bx, by, bz);
        if (!b) return 0;
        return b->skyLight;
    }

    void SetSky(int bx, int by, int bz, uint8_t light) const {
        Block* b = GetBlockPtr(bx, by, bz);
        if (!b) return;
        b->skyLight = light;
    }

    bool IsTransparent(int bx, int by, int bz) const {
        unsigned int id = GetBlockId(bx, by, bz);
        return id == 0 ||
            id == (unsigned int)BlockType::Water ||
            id == (unsigned int)BlockType::Leave;
    }

    uint8_t ComputeAttenuation(unsigned int id) const {
        if (id == 0) return 1;
        if (id == (unsigned int)BlockType::Water) return 2;
        if (id == (unsigned int)BlockType::Leave) return 3;
        return 255;
    }
};