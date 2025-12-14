//
// Created by vivek on 11/5/2025.
//
#include "bvh.h"
#include "renderer.h"
#include "tracy/Tracy.hpp"

void BVH::buildFromScene(const std::vector<TriangleMesh> &scene, int leafSize) {
  ZoneScopedN("BuildBVH");

  // flatten scene -> tri/triIdx
  size_t N = 0;
  for (auto &M : scene)
    N += M.tris.size();
  tri.clear();
  tri.resize(N);
  triIdx.clear();
  triIdx.resize(N);
  nodes.clear();
  nodes.resize(std::max(1, static_cast<int>(2*N)));
  nodesUsed = 1;

  size_t w = 0;
  for (int m = 0; m < (int)scene.size(); ++m) {
    const TriangleMesh &M = scene[m];
    for (int t = 0; t < (int)M.tris.size(); ++t, ++w) {
      const Triangle &T = M.tris[t];
      BVHTri &dst = tri[w];
      dst.vertex0 = M.positions[T.i0];
      dst.vertex1 = M.positions[T.i1];
      dst.vertex2 = M.positions[T.i2];
      dst.centroid = (dst.vertex0 + dst.vertex1 + dst.vertex2) * (1.0f / 3.0f);
      dst.mesh = m;
      dst.triId = t;
      dst.material = T.material;
      triIdx[w] = (uint32_t)w;
    }
  }

  BVHNode &root = nodes[0];
  root.leftFirst = 0;
  root.triCount = (uint32_t)N;
  UpdateNodeBounds(0);
  SubdivideSAH(0, leafSize);
  nodes.resize(nodesUsed);
}

void BVH::UpdateNodeBounds(uint32_t nodeIdx) {
  // ZoneScopedN("UpdateNodeBounds");
  BVHNode &node = nodes[nodeIdx];
  node.aabbMin = float3(1e30f);
  node.aabbMax = float3(-1e30f);
  for (uint32_t first = node.leftFirst, i = 0; i < node.triCount; i++) {
    uint32_t leafTriIdx = triIdx[first + i];
    const BVHTri &leafTri = tri[leafTriIdx];
    node.aabbMin = fminf(node.aabbMin, leafTri.vertex0);
    node.aabbMin = fminf(node.aabbMin, leafTri.vertex1);
    node.aabbMin = fminf(node.aabbMin, leafTri.vertex2);
    node.aabbMax = fmaxf(node.aabbMax, leafTri.vertex0);
    node.aabbMax = fmaxf(node.aabbMax, leafTri.vertex1);
    node.aabbMax = fmaxf(node.aabbMax, leafTri.vertex2);
  }
}

float BVH::EvaluateSAH(BVHNode &node, int axis, float pos) {
  aabb leftBox, rightBox;
  int leftCount = 0, rightCount = 0;
  for (uint32_t i = 0; i < node.triCount; i++) {
    const BVHTri &triangle = tri[triIdx[node.leftFirst + i]];
    if ((&triangle.centroid.x)[axis] < pos) {
      leftCount++;
      leftBox.grow(triangle.vertex0);
      leftBox.grow(triangle.vertex1);
      leftBox.grow(triangle.vertex2);
    } else {
      rightCount++;
      rightBox.grow(triangle.vertex0);
      rightBox.grow(triangle.vertex1);
      rightBox.grow(triangle.vertex2);
    }
  }
  float cost = leftCount * leftBox.area() + rightCount * rightBox.area();
  return cost > 0 ? cost : 1e30f;
}

float BVH::FindBestSplitPlane(BVHNode &node, int &axis, float &splitPos) {
  float bestCost = 1e30f;
  for (int a = 0; a < 3; a++) {
    float boundsMin = 1e30f, boundsMax = -1e30f;
    for (uint32_t i = 0; i < node.triCount; i++) {
      const BVHTri &triangle = tri[triIdx[node.leftFirst + i]];
      float c = (&triangle.centroid.x)[a];
      boundsMin = std::min(boundsMin, c);
      boundsMax = std::max(boundsMax, c);
    }
    if (boundsMin == boundsMax)
      continue;

    struct Bin {
      aabb bounds;
      int triCount = 0;
    };
    Bin bin[BINS];
    float scale = float(BINS) / (boundsMax - boundsMin);

    for (uint32_t i = 0; i < node.triCount; i++) {
      const BVHTri &triangle = tri[triIdx[node.leftFirst + i]];
      int binIdx = std::min(
          BINS - 1, int(((&triangle.centroid.x)[a] - boundsMin) * scale));
      bin[binIdx].triCount++;
      bin[binIdx].bounds.grow(triangle.vertex0);
      bin[binIdx].bounds.grow(triangle.vertex1);
      bin[binIdx].bounds.grow(triangle.vertex2);
    }

    float leftArea[BINS - 1], rightArea[BINS - 1];
    int leftCount[BINS - 1], rightCount[BINS - 1];
    aabb leftBox, rightBox;
    int leftSum = 0, rightSum = 0;

    for (int i = 0; i < BINS - 1; i++) {
      leftSum += bin[i].triCount;
      leftCount[i] = leftSum;
      leftBox.grow(bin[i].bounds);
      leftArea[i] = leftBox.area();

      rightSum += bin[BINS - 1 - i].triCount;
      rightCount[BINS - 2 - i] = rightSum;
      rightBox.grow(bin[BINS - 1 - i].bounds);
      rightArea[BINS - 2 - i] = rightBox.area();
    }

    float step = (boundsMax - boundsMin) / float(BINS);
    for (int i = 0; i < BINS - 1; i++) {
      float planeCost =
          leftCount[i] * leftArea[i] + rightCount[i] * rightArea[i];
      if (planeCost < bestCost) {
        axis = a;
        splitPos = boundsMin + step * (i + 1);
        bestCost = planeCost;
      }
    }
  }
  return bestCost;
}

float BVH::calculateNodeCost(BVHNode &node) {
  float3 e = node.aabbMax - node.aabbMin;
  float surfaceArea = e.x * e.y + e.y * e.z + e.z * e.x;
  return node.triCount * surfaceArea;
}

void BVH::SubdivideSAH(uint32_t nodeIdx, int leafSize) {
  // ZoneScopedN("SubdivideSAH");
  BVHNode &node = nodes[nodeIdx];

  if ((int)node.triCount <= leafSize)
    return;

  int axis = 0;
  float splitPos = 0.0f;
  float splitCost = FindBestSplitPlane(node, axis, splitPos);
  float nosplitCost = calculateNodeCost(node);
  if (splitCost >= nosplitCost)
    return;

  // partition in-place
  uint32_t i = node.leftFirst;
  uint32_t j = i + node.triCount - 1;
  while (i <= j) {
    if ((&tri[triIdx[i]].centroid.x)[axis] < splitPos)
      i++;
    else
      std::swap(triIdx[i], triIdx[j--]);
  }

  int leftCount = int(i) - int(node.leftFirst);
  if (leftCount == 0 || leftCount == int(node.triCount))
    return;

  uint32_t leftChildIdx = nodesUsed++;
  uint32_t rightChildIdx = nodesUsed++;
  nodes[leftChildIdx].leftFirst = node.leftFirst;
  nodes[leftChildIdx].triCount = leftCount;
  nodes[rightChildIdx].leftFirst = i;
  nodes[rightChildIdx].triCount = node.triCount - leftCount;

  node.leftFirst = leftChildIdx;
  node.triCount = 0; // internal

  UpdateNodeBounds(leftChildIdx);
  UpdateNodeBounds(rightChildIdx);

  SubdivideSAH(leftChildIdx, leafSize);
  SubdivideSAH(rightChildIdx, leafSize);
}

static inline bool hitAABB(const float3 &bmin, const float3 &bmax, const Ray &r,
                           float tmax) {
  const float3 invD = {1.0f / r.d.x, 1.0f / r.d.y, 1.0f / r.d.z};
  const float3 t0 = (bmin - r.o) * invD;
  const float3 t1 = (bmax - r.o) * invD;
  const float3 tminv = {std::min(t0.x, t1.x), std::min(t0.y, t1.y),
                        std::min(t0.z, t1.z)};
  const float3 tmaxv = {std::max(t0.x, t1.x), std::max(t0.y, t1.y),
                        std::max(t0.z, t1.z)};
  float tmin = std::max(std::max(tminv.x, tminv.y), std::max(0.0f, tminv.z));
  float tmax2 = std::min(std::min(tmaxv.x, tmaxv.y), tmaxv.z);
  return (tmax2 >= tmin) && (tmin < tmax);
}

bool BVH::intersect(const Ray &r, Hit &h) const {
  // ZoneScopedN("Traversal");
  if (nodesUsed == 0)
    return false;

  bool any = false;
  int stack[64];
  int sp = 0;
  stack[sp++] = 0; // root

  while (sp) {
    const BVHNode &n = nodes[stack[--sp]];
    if (!hitAABB(n.aabbMin, n.aabbMax, r, h.t))
      continue;

    if (n.isLeaf()) {
      const uint32_t first = n.leftFirst;
      const uint32_t last = first + n.triCount;
      for (uint32_t k = first; k < last; ++k) {
        const BVHTri &T = tri[triIdx[k]];
        float t, u, v;
        if (intersectTriangle(r, T.vertex0, T.vertex1, T.vertex2, t, u, v) &&
            t < h.t) {
          h.t = t;
          h.u = u;
          h.v = v;
          h.mesh = T.mesh;
          h.tri = T.triId;
          h.found = true;
          any = true;
        }
      }
    } else {
      const int left = (int)n.leftFirst;
      const int right = left + 1;
      // (optional) near-first push using ray dir & split axis; omitted for now
      stack[sp++] = right;
      stack[sp++] = left;
    }
  }
  return any;
}
