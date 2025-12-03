//
// Created by vivek on 10/31/2025.
//
#include <cfloat>
#include <vector>
#include <string>
#include "vmath.h"

#ifndef MESH_H
#define MESH_H

struct Ray {
  float3 o, d;
};

struct Hit {
  bool found = false;
  double t = FLT_MAX;
  float3 n;
  float u = 0, v = 0;
  int tri = -1, mesh = -1;
};

struct Tri {
  int i0, i1, i2;
  int n0, n1, n2;
  int uv0, uv1, uv2;
};

struct Triangle {
  int i0, i1, i2;
  int n0, n1, n2;
  int material;
};

struct Mesh {
  std::vector<float3> positions;
  std::vector<float3> normals;
  std::vector<float3> uvs;
  std::vector<Tri> tris;
  // float3              albedo = float3(0.8);
};

struct TriangleMesh {
  std::vector<float3> positions;
  std::vector<float3> normals;
  std::vector<float3> uvs;
  std::vector<Triangle> tris;
  // float3              albedo = float3(0.8);
};

struct Materials {
  std::vector<float3> Kd_linear;
  std::vector<float> alpha;
};

struct AABB {
  float3 minv = {+FLT_MAX, +FLT_MAX, +FLT_MAX};
  float3 maxv = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

  void expand(const float3 &p) {
    minv.x = __min(minv.x, p.x);
    minv.y = __min(minv.y, p.y);
    minv.z = __min(minv.z, p.z);

    maxv.x = __max(maxv.x, p.x);
    maxv.y = __max(maxv.y, p.y);
    maxv.z = __max(maxv.z, p.z);
  }

  float3 center() const { return (minv + maxv) * 0.5f; }
  float3 halfExtent() const { return (maxv - minv) * 0.5f; }
  float radius() const { return length(halfExtent()); }
};
#endif // MESH_H
