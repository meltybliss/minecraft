#pragma once
#include "Vec3.h"

struct Ray {
	Vec3 origin;
	Vec3 dir;

	Vec3 At(float t) const {
		return origin + dir * t;
	}

};