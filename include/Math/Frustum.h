#pragma once
#include "Vec3.h"
#include "Rendering/camera.h"
struct Plane {
	Vec3 n;
	float d;
};

struct Frustum {
	Plane planes[6];
};

Plane MakePlane(const Vec3& a, const Vec3& b, const Vec3& c);
Frustum BuildFrustumFromCamera(const Camera& cam);
Plane MakePlaneFacingInside(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& insidePoint);
bool IsAABBOutsidePlane(const Plane& p, const Vec3& boxMin, const Vec3& boxMax);
bool IsAABBVisible(const Frustum& fr, const Vec3& boxMin, const Vec3& boxMax);