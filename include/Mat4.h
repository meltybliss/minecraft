#pragma once
#include <cmath>
#include "camera.h"

struct Mat4 {
	float m[16]{};

	static Mat4 Identity() {
		Mat4 r{};
		r.m[0] = 1.0f;
		r.m[5] = 1.0f;
		r.m[10] = 1.0f;
		r.m[15] = 1.0f;
		return r;
	}

};

inline Mat4 Perspective(float fovDeg, float aspect, float nearZ, float farZ) {
    Mat4 r{};

    float fovRad = fovDeg * 3.14159265f / 180.0f;
    float f = 1.0f / std::tan(fovRad * 0.5f);

    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (farZ + nearZ) / (nearZ - farZ);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);

    return r;
}

inline float Dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = Normalize(center - eye);
    Vec3 s = Normalize(Cross(f, up));
    Vec3 u = Cross(s, f);

    Mat4 r = Mat4::Identity();

    r.m[0] = s.x;
    r.m[4] = s.y;
    r.m[8] = s.z;

    r.m[1] = u.x;
    r.m[5] = u.y;
    r.m[9] = u.z;

    r.m[2] = -f.x;
    r.m[6] = -f.y;
    r.m[10] = -f.z;

    r.m[12] = -Dot(s, eye);
    r.m[13] = -Dot(u, eye);
    r.m[14] = Dot(f, eye);

    return r;
}