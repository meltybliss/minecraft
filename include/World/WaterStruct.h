#pragma once
#include "BlockPos.h"
#include <memory>

namespace WaterDef {
	constexpr int initialWaterLevel = 35;
}

struct WaterData {
	BlockPos pos{0, 0, 0};
	std::shared_ptr<int> level;
};