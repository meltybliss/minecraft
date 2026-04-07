#pragma once
#include "BlockPos.h"
#include <memory>
struct WaterData {
	BlockPos pos{0, 0, 0};
	std::shared_ptr<int> level;
};