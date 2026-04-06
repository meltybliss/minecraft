#include "Frustum.h"

Plane MakePlane(const Vec3& a, const Vec3& b, const Vec3& c) {
    Vec3 ab = b - a;
    Vec3 ac = c - a;
    Vec3 n = Normalize(Cross(ab, ac));
    float d = -Dot(n, a);
    return { n, d };
}


Frustum BuildFrustumFromCamera(const Camera& cam) {
    Frustum fr{};

    float fovRad = cam.fovDeg * 3.14159265f / 180.0f;
    float tanHalf = std::tan(fovRad * 0.5f);

    Vec3 nc = cam.pos + cam.forward * cam.nearZ;
    Vec3 fc = cam.pos + cam.forward * cam.farZ;

    float nearHalfH = tanHalf * cam.nearZ;
    float nearHalfW = nearHalfH * cam.aspect;

    float farHalfH = tanHalf * cam.farZ;
    float farHalfW = farHalfH * cam.aspect;

    Vec3 ntl = nc + cam.up * nearHalfH - cam.right * nearHalfW;
    Vec3 ntr = nc + cam.up * nearHalfH + cam.right * nearHalfW;
    Vec3 nbl = nc - cam.up * nearHalfH - cam.right * nearHalfW;
    Vec3 nbr = nc - cam.up * nearHalfH + cam.right * nearHalfW;

    Vec3 ftl = fc + cam.up * farHalfH - cam.right * farHalfW;
    Vec3 ftr = fc + cam.up * farHalfH + cam.right * farHalfW;
    Vec3 fbl = fc - cam.up * farHalfH - cam.right * farHalfW;
    Vec3 fbr = fc - cam.up * farHalfH + cam.right * farHalfW;

    Vec3 insidePoint = cam.pos + cam.forward * ((cam.nearZ + cam.farZ) * 0.5f);

    fr.planes[0] = MakePlaneFacingInside(nbl, ntl, ftl, insidePoint); // left
    fr.planes[1] = MakePlaneFacingInside(ntr, nbr, fbr, insidePoint); // right
    fr.planes[2] = MakePlaneFacingInside(ntl, ntr, ftr, insidePoint); // top
    fr.planes[3] = MakePlaneFacingInside(nbr, nbl, fbl, insidePoint); // bottom
    fr.planes[4] = MakePlaneFacingInside(ntl, nbl, nbr, insidePoint); // near
    fr.planes[5] = MakePlaneFacingInside(fbr, fbl, ftl, insidePoint); // far

    return fr;
}


bool IsAABBOutsidePlane(const Plane& p, const Vec3& boxMin, const Vec3& boxMax) {
    Vec3 positive = boxMin;

    if (p.n.x >= 0) positive.x = boxMax.x;
    if (p.n.y >= 0) positive.y = boxMax.y;
    if (p.n.z >= 0) positive.z = boxMax.z;

    float dist = Dot(p.n, positive) + p.d;
    return dist < 0.0f;
}

bool IsAABBVisible(const Frustum& fr, const Vec3& boxMin, const Vec3& boxMax) {
    for (int i = 0; i < 6; i++) {
        if (IsAABBOutsidePlane(fr.planes[i], boxMin, boxMax)) {
            return false;
        }
    }
    return true;
}

Plane MakePlaneFacingInside(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& insidePoint) {
    Vec3 ab = b - a;
    Vec3 ac = c - a;
    Vec3 n = Normalize(Cross(ab, ac));
    float d = -Dot(n, a);

    // insidePoint ‚ª•‰‘¤‚É‚ ‚é‚È‚ç”½“]
    if (Dot(n, insidePoint) + d < 0.0f) {
        n = n * -1.0f;
        d = -d;
    }

    return { n, d };
}