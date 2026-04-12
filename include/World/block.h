#pragma once
#include <stdint.h>

enum class BlockType {
	AIR = 0,
	Grass,
	Dirt,
	Stone,
	Ore,
	Wood,
	Leave,
	TNT,
	Water,
	Sand,
};


struct Block {
	unsigned int type = 0;
	uint8_t skyLight = 0;
	uint8_t blockLight = 0;

};

extern int blockSize;