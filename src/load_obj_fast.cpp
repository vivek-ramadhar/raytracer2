#include <vector>
// #include <xstring>
#include <cstring>
#include <string>

#include "mesh.h"
#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"
#endif


//
// Created by vivek on 10/31/2025.
//

#include "load_obj_fast.h"
extern "C" {
#include "fast_obj.h"
}

static inline float3 P3(const float *p) { return float3(p[0], p[1], p[2]); }
static inline float3 N3(const float *n) { return float3(n[0], n[1], n[2]); }
static inline float3 T2(const float *t) { return float3(t[0], t[1], 0.0f); }

bool load_meshes_fastobj(const std::string &path, std::vector<Mesh> &out,
                         AABB &sceneBounds) {
  fastObjMesh *m = fast_obj_read(path.c_str());
  if (!m)
    return false;

  std::vector<float3> P(m->position_count);
  for (unsigned i = 0; i < m->position_count; ++i) {
    P[i] = P3(&m->positions[3 * i]);
    sceneBounds.expand(P[i]);
  }

  std::vector<float3> N(m->normal_count);
  for (unsigned i = 0; i < m->normal_count; ++i)
    N[i] = N3(&m->normals[3 * i]);

  std::vector<float3> T(m->texcoord_count);
  for (unsigned i = 0; i < m->texcoord_count; ++i)
    T[i] = T2(&m->texcoords[2 * i]);

  out.clear();
  out.reserve(m->group_count);

  for (unsigned g = 0; g < m->group_count; ++g) {
    const fastObjGroup &G = m->groups[g];
    Mesh M;
    M.positions = P;
    M.normals = N;
    M.uvs = T;
    M.tris.reserve(G.face_count);

    size_t idx = G.index_offset;
    for (unsigned f = 0; f < G.face_count; ++f) {
      const unsigned corners = m->face_vertices[G.face_offset + f];
      const fastObjIndex v0 = m->indices[idx + 0];
      for (unsigned k = 1; k + 1 < corners; ++k) {
        const fastObjIndex v1 = m->indices[idx + k];
        const fastObjIndex v2 = m->indices[idx + k + 1];
        Tri t{};
        t.i0 = v0.p, t.i1 = v1.p, t.i2 = v2.p;
        t.n0 = v0.n, t.n1 = v1.n, t.n2 = v2.n;
        t.uv0 = v0.t, t.uv1 = v1.t, t.uv2 = v2.t;
        M.tris.push_back(t);
      }
      idx += corners;
    }
    out.push_back(std::move(M));
  }
  fast_obj_destroy(m);
  return true;
}

bool load_triangle_meshes_fastobj(const std::string &path,
                                  std::vector<TriangleMesh> &outMeshes,
                                  Materials &outMats, AABB &sceneBounds) {
  #ifdef TRACY_ENABLE
    ZoneScopedN("load_triangle_mesh_fastobj");
  #endif
  fastObjMesh *om = fast_obj_read(path.c_str());
  if (!om)
    return false;

  outMats.Kd_linear.resize(om->material_count);
  outMats.alpha.resize(om->material_count, 1.0f);


  for (unsigned m = 0; m < om->material_count; ++m) {
    const fastObjMaterial& mat = om->materials[m];
    float3 kd = {mat.Kd[0], mat.Kd[1], mat.Kd[2]};
    outMats.Kd_linear[m] = srgb::to_linear(kd);
    outMats.alpha[m] = mat.d;
  }

  TriangleMesh M;
  M.positions.reserve(om->position_count);
  for (unsigned i = 0; i < om->position_count; ++i) {
    float3 p{(float)om->positions[3 * i + 0], (float)om->positions[3 * i + 1],
             (float)om->positions[3 * i + 2]};
    sceneBounds.expand(p);
    M.positions.push_back(p);
  }

  M.normals.reserve(om->normal_count);
  for (unsigned i = 0; i < om->normal_count; ++i) {
    float3 n{(float)om->normals[3 * i + 0], (float)om->normals[3 * i + 1],
             (float)om->normals[3 * i + 2]};
    // sceneBounds.expand(p);
    M.normals.push_back(n);
  }

  size_t cur = 0;
  M.tris.reserve(om->face_count);
  for (unsigned f = 0; f < om->face_count; ++f) {
    const fastObjIndex i0 = om->indices[cur + 0];
    const fastObjIndex i1 = om->indices[cur + 1];
    const fastObjIndex i2 = om->indices[cur + 2];
    cur += 3;

    Triangle T{};
    T.i0 = i0.p;
    T.i1 = i1.p;
    T.i2 = i2.p;
    T.n0 = i0.n;
    T.n1 = i1.n;
    T.n2 = i2.n;
    T.material = om->face_materials ? om->face_materials[f] : -1;

    M.tris.push_back(T);
  }

  outMeshes.push_back(std::move(M));
  fast_obj_destroy(om);
  return true;
}
