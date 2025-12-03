//
// Created by vivek on 10/31/2025.
//

#include "bvh.h"
#include "mesh.h"
#include "tracy/Tracy.hpp"
#include <algorithm>

#include "sh_lighting.h"

inline bool intersectTriMT(const Ray& r, const float3& p0, const float3& p1, const float3& p2,
                           double& t, double& u, double& v) {
    const float3 e1 = p1 - p0, e2 = p2 - p0;
    const float3 p = cross(r.d, e2);
    const double det = dot(e1, p);
    if (fabs(det) < 1e-12) return false;
    const double invDet = 1.0/det;
    const float3 s = r.o - p0;
    u = dot(s, p)*invDet; if (u < 0.0 || u > 1.0) return false;
    const float3 q = cross(s, e1);
    v = dot(r.d, q)*invDet; if (v < 0.0 || (u+v) > 1.0) return false;
    t = dot(e2, q)*invDet; if (t <= 1e-6) return false;
    return true;
}

inline bool intersectMeshLinear(const Mesh& M, const Ray& r, Hit& h) {
    bool any = false;
    for (int i = 0; i < (int)M.tris.size(); ++i) {
        const Tri& T = M.tris[i];
        double t,u,v;
        if (intersectTriMT(r, M.positions[T.i0], M.positions[T.i1], M.positions[T.i2], t, u, v) && t < h.t) {
            h.t = t; h.tri = i; any = true;
            if (T.n0 >= 0 && T.n1 >= 0 && T.n2 >= 0 && !M.normals.empty()) {
                float3 n = (1-u-v)*M.normals[T.n0] + u*M.normals[T.n1] + v*M.normals[T.n2];
                h.n = normalize(n);
            } else {
                h.n = normalize(cross(M.positions[T.i1] - M.positions[T.i0],
                                        M.positions[T.i2] - M.positions[T.i0]));
            }
        }
    }
    return any;
}

inline bool intersectTriangle(const Ray& r, const float3& v0, const float3& v1, const float3& v2,
                              float& t, float& u, float& v) {
    const float3 e1 = v1 - v0;
    const float3 e2 = v2 - v0;
    const float3 p = cross(r.d, e2);
    const float det = dot(e1, p);
    if (fabsf(det) < 1e-8f) return false;
    const float invDet = 1.0f / det;

    const float3 s = r.o - v0;
    u = dot(s, p) * invDet;
    if (u < 0.0f || u > 1.0f) return false;

    const float3 q = cross(s, e1);
    v = dot(r.d, q) * invDet;
    if (v < 0.0f || (u + v) > 1.0f) return false;

    t = dot(e2, q) * invDet;
    return t > 1e-5f;
}

inline bool intersectSceneLinear(const std::vector<TriangleMesh>& scene, const Ray& r, Hit& hit) {
    bool any = false;
    for (int mi = 0; mi < (int)scene.size(); ++mi) {
        const TriangleMesh& M = scene[mi];
        for (int ti = 0; ti < (int)M.tris.size(); ++ti) {
            const Triangle& T = M.tris[ti];
            const float3& v0 = M.positions[T.i0];
            const float3& v1 = M.positions[T.i1];
            const float3& v2 = M.positions[T.i2];

            float t, u, v;
            if (!intersectTriangle(r, v0, v1, v2, t, u, v)) continue;
            if (t < hit.t) {
                hit.t = t; hit.u = u; hit.v = v;
                hit.mesh = mi; hit.tri = ti;
                hit.found = true;
                any = true;
            }
        }
    }
    return (any || hit.found);
}

inline float3 interpolateNormal(const TriangleMesh& M, const Triangle& T, float u, float v) {
    const float w = 1.0f - u - v;

    if (T.n0 >= 0 && T.n1 >= 0 && T.n2 >= 0 &&
        T.n0 < (int)M.normals.size() &&
        T.n1 < (int)M.normals.size() &&
        T.n2 < (int)M.normals.size()) {

        const float3 n0 = M.normals[T.n0];
        const float3 n1 = M.normals[T.n1];
        const float3 n2 = M.normals[T.n2];
        return normalize(n0*w + n1*u + n2*v);
    }

    const float3& p0 = M.positions[T.i0];
    const float3& p1 = M.positions[T.i1];
    const float3& p2 = M.positions[T.i2];
    return normalize(cross(p1 - p0, p2 - p0));
}

struct Camera {
    float3 pos, dir, right, up, forward;
    double fovY_deg = 45.0;

    float fovy;
    float aspect;
    float near_plane, far_plane;
};

static inline uint8_t to8(double x) {
    int v = (int)std::round(std::clamp(x, 0.0, 1.0) * 255.0);
    return (uint8_t)v;
}

static inline float3 sky(const Ray& r) {
    double t = 0.5*(normalize(r.d).y+1.0);
    return (1.0 - t)*float3(1, 1, 1) + t*float3(0.5, 0.7, 1.0);
}


void render_bgr24(const std::vector<TriangleMesh>& scene, const Materials& materials, const Camera& cam,
    int W, int H, unsigned char* bgr);

void render_bgr24(const std::vector<Mesh>& scene, const Camera& cam, int W, int H, unsigned char* out);

void render_bgr24(const std::vector<TriangleMesh>& scene, const BVH& bvh, const Materials& materials, const Camera& cam,
                  int W, int H, unsigned char* bgr);

void render_bgr24(const std::vector<TriangleMesh>& scene, const BVH& bvh, const Materials& materials,
                  const SHContext& sh, const Camera& cam, int W, int H, unsigned char* bgr);
