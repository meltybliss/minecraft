#pragma once

struct BlockPos {

	int x;
	int y;
	int z;

	BlockPos operator+(const BlockPos& b) const {
		return { x + b.x, y + b.y, z + b.z };
	}

};