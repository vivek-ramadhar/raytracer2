//
// Created by vivek on 11/5/2025.
//
#include "mesh.h"
#include "tracy/Tracy.hpp"
#include "vmath.h"

#ifndef BVH_H
#define BVH_H

#ifndef RAYTRACER_ALIGN
#if defined(_MSC_VER)
#define RAYTRACER_ALIGN(x) __declspec(align(x))
#else
#define RAYTRACER_ALIGN(x) __attribute__((aligned(x)))
#endif
#endif

RAYTRACER_ALIGN(64) struct BVHNode {
  struct {
    float3 aabbMin;
    uint32_t leftFirst;
  };
  struct {
    float3 aabbMax;
    uint32_t triCount;
  };
  bool isLeaf() const { return triCount > 0; }
};

struct BVHTri {
  float3 vertex0, vertex1, vertex2;
  float3 centroid;
  int mesh = -1;
  int triId = -1;
  int material = -1;
};

class BVH {
public:
  void buildFromScene(const std::vector<TriangleMesh> &scene, int leafSize = 4);
  bool intersect(const Ray &r, Hit &h) const;

  std::vector<BVHNode> nodes;
  std::vector<uint32_t> triIdx;
  std::vector<BVHTri> tri;

private:
  uint32_t nodesUsed = 0;
  static constexpr int BINS = 16;

  struct aabb {
    float3 bmin = 1e30f, bmax = -1e30f;
    void grow(float3 p) { bmin = fminf(bmin, p), bmax = fmaxf(bmax, p); }
    void grow(aabb &b) {
      if (b.bmin.x != 1e30f) {
        grow(b.bmin);
        grow(b.bmax);
      }
    }
    float area() {
      float3 e = bmax - bmin; // extent
      return e.x * e.y + e.y * e.z + e.z * e.x;
    }
  };

  void UpdateNodeBounds(uint32_t nodeIdx);
  float EvaluateSAH(BVHNode &node, int axis, float pos);
  float FindBestSplitPlane(BVHNode &, int &axis, float &splitPos);
  float calculateNodeCost(BVHNode &node);
  void SubdivideSAH(uint32_t nodeIdx, int leafSize);
};

#endif // BVH_H
