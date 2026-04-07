#pragma once
#include "Math/Vec3.h"
struct HitResult {
	bool isHit = false;
	Vec3 hitPos;
	Vec3 normal;
	float dist;

};