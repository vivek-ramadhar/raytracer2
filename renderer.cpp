#include "renderer.h"

#include <iostream>
#include <vector>

#include "mesh.h"
#include "sh_lighting.h"
#include "tracy/Tracy.hpp"
//
// Created by vivek on 11/5/2025.
//
void render_bgr24(const std::vector<TriangleMesh> &scene,
                  const Materials &materials, const Camera &cam, int W, int H,
                  unsigned char *bgr) {
  ZoneScopedN("render_bgr24");
  const float tanHalfFovY = tanf(cam.fovy * 0.5f);
  const float sx = cam.aspect * tanHalfFovY;
  const float sy = tanHalfFovY;

  auto write = [&](int x, int y, const float3 &c_lin) {
    float r = srgb::from_linear(clamp(c_lin.x, 0.0f, 1.0f));
    float g = srgb::from_linear(clamp(c_lin.y, 0.0f, 1.0f));
    float b = srgb::from_linear(clamp(c_lin.z, 0.0f, 1.0f));
    int idx = 3 * (y * W + x);
    bgr[idx + 0] = (unsigned char)std::round(255.0f * b);
    bgr[idx + 1] = (unsigned char)std::round(255.0f * g);
    bgr[idx + 2] = (unsigned char)std::round(255.0f * r);
  };

  for (int y = 0; y < H; ++y) {
    for (int x = 0; x < W; ++x) {
      double u = ((x + 0.5) / double(W) * 2.0 - 1.0) * sx;
      double v = ((y + 0.5) / double(H) * 2.0 - 1.0) * sy;
      v = -v;

      Ray r;
      r.o = cam.pos;
      r.d = normalize(cam.forward + float(u) * cam.right + float(v) * cam.up);

      Hit h;
      {
        ZoneScopedN("intersectSceneLinear");
        if (intersectSceneLinear(scene, r, h)) {
          const TriangleMesh &M = scene[h.mesh];
          const Triangle &T = M.tris[h.tri];

          float3 n = interpolateNormal(M, T, h.u, h.v);

          const float3 &p0 = M.positions[T.i0];
          const float3 &p1 = M.positions[T.i1];
          const float3 &p2 = M.positions[T.i2];
          float3 ng = normalize(cross(p1 - p0, p2 - p0));

          if (dot(n, ng) < 0.0f)
            n = -n;
          // if (dot(n, r.d) > 0.0f) n = -n;

          float3 albedo = {0.8f, 0.8f, 0.8f};
          if (T.material >= 0 && T.material < (int)materials.Kd_linear.size())
            albedo = materials.Kd_linear[T.material];

          const float3 L = normalize(float3{0.4f, 0.6f, 1.0f});
          float ndotl = __max(0.0f, dot(n, L));
          float3 color = albedo * ndotl + 0.05f * albedo;
          write(x, y, color);
        } else {
          write(x, y, float3{0.95f, 0.97f, 1.0f}); // background
        }
      }
    }
  }
}

void render_bgr24(const std::vector<Mesh> &scene, const Camera &cam, int W,
                  int H, unsigned char *out) {
  const double fov = cam.fovY_deg * (3.141592653589793 / 180.0);
  const double sy = std::tan(0.5 * fov);
  const double sx = sy * (double)W / (double)H;

  int loop_ctr = 0;
  for (int y = 0; y < H; ++y) {
    for (int x = 0; x < W; ++x) {
      double u = ((x + 0.5) / W * 2.0 - 1.0) * sx;
      double v = -((y + 0.5) / H * 2.0 - 1.0) * sy;
      Ray r;
      r.o = cam.pos;
      r.d = normalize(u * cam.right + v * cam.up + cam.forward);

      Hit best;
      bool any = false;
      int mi = -1;
      for (int m = 0; m < (int)scene.size(); ++m) {
        Hit h;
        if (intersectMeshLinear(scene[m], r, h) && h.t < best.t) {
          best = h;
          any = true;
          mi = m;
        }
      }

      float3 albedo = float3(1, 1, 1);
      float3 C =
          any ? (0.5 * (best.n + float3(1, 1, 1)) * albedo /*scene[mi].albedo*/)
              : sky(r);
      unsigned char *px = &out[(y * W + x) * 3];
      px[0] = to8(C.z);
      px[1] = to8(C.y);
      px[2] = to8(C.x);
    }
  }
}

void render_bgr24(const std::vector<TriangleMesh> &scene, const BVH &bvh,
                  const Materials &materials, const Camera &cam, int W, int H,
                  unsigned char *bgr) {
  // ZoneScopedN("render_bgr24");
  const float tanHalfFovY = tanf(cam.fovy * 0.5f);
  const float sx = cam.aspect * tanHalfFovY;
  const float sy = tanHalfFovY;

  auto write = [&](int x, int y, const float3 &c_lin) {
    float r = srgb::from_linear(clamp(c_lin.x, 0.0f, 1.0f));
    float g = srgb::from_linear(clamp(c_lin.y, 0.0f, 1.0f));
    float b = srgb::from_linear(clamp(c_lin.z, 0.0f, 1.0f));
    int idx = 3 * (y * W + x);
    bgr[idx + 0] = (unsigned char)std::round(255.0f * b);
    bgr[idx + 1] = (unsigned char)std::round(255.0f * g);
    bgr[idx + 2] = (unsigned char)std::round(255.0f * r);
  };

  for (int y = 0; y < H; ++y) {
    for (int x = 0; x < W; ++x) {
      double u = ((x + 0.5) / double(W) * 2.0 - 1.0) * sx;
      double v = ((y + 0.5) / double(H) * 2.0 - 1.0) * sy;
      v = -v;

      Ray r;
      r.o = cam.pos;
      r.d = normalize(cam.forward + float(u) * cam.right + float(v) * cam.up);

      Hit h;
      {
        // ZoneScopedN("bvh.intersect(r, h)");
        if (bvh.intersect(r, h)) {
          const TriangleMesh &M = scene[h.mesh];
          const Triangle &T = M.tris[h.tri];

          float3 n = interpolateNormal(M, T, h.u, h.v);

          const float3 &p0 = M.positions[T.i0];
          const float3 &p1 = M.positions[T.i1];
          const float3 &p2 = M.positions[T.i2];
          float3 ng = normalize(cross(p1 - p0, p2 - p0));

          if (dot(n, ng) < 0.0f)
            n = -n;
          // if (dot(n, r.d) > 0.0f) n = -n;

          float3 albedo = {0.8f, 0.8f, 0.8f};
          if (T.material >= 0 && T.material < (int)materials.Kd_linear.size())
            albedo = materials.Kd_linear[T.material];

          const float3 L = normalize(float3{0.4f, 0.6f, 1.0f});
          float ndotl = __max(0.0f, dot(n, L));
          float3 color = albedo * ndotl + 0.05f * albedo;
          write(x, y, color);
        } else {
          write(x, y, float3{0.95f, 0.97f, 1.0f}); // background
        }
      }
    }
  }
}

void render_bgr24(const std::vector<TriangleMesh> &scene, const BVH &bvh,
                  const Materials &materials, const SHContext &sh,
                  const Camera &cam, int W, int H, unsigned char *bgr) {
  const float tanHalfFovY = tanf(cam.fovy * 0.5f);
  const float sx = cam.aspect * tanHalfFovY;
  const float sy = tanHalfFovY;

  auto write = [&](int x, int y, const float3 &c_lin) {
    float r = srgb::from_linear(clamp(c_lin.x, 0.0f, 1.0f));
    float g = srgb::from_linear(clamp(c_lin.y, 0.0f, 1.0f));
    float b = srgb::from_linear(clamp(c_lin.z, 0.0f, 1.0f));
    int idx = 3 * (y * W + x);
    bgr[idx + 0] = (unsigned char)std::round(255.0f * b);
    bgr[idx + 1] = (unsigned char)std::round(255.0f * g);
    bgr[idx + 2] = (unsigned char)std::round(255.0f * r);
  };

  for (int y = 0; y < H; ++y) {
    for (int x = 0; x < W; ++x) {
      double u = ((x + 0.5) / double(W) * 2.0 - 1.0) * sx;
      double v = ((y + 0.5) / double(H) * 2.0 - 1.0) * sy;
      v = -v;

      Ray r;
      r.o = cam.pos;
      r.d = normalize(cam.forward + float(u) * cam.right + float(v) * cam.up);

      Hit h;
      if (bvh.intersect(r, h)) {
        const TriangleMesh &M = scene[h.mesh];
        const Triangle &T = M.tris[h.tri];

        float b1 = h.u;
        float b2 = h.v;
        float b0 = 1.0f - b1 - b2;

        int i0 = T.i0;
        int i1 = T.i1;
        int i2 = T.i2;

        // simple palette by material id
        // const float3 matColors[] = {
        // {1, 0, 0},   // material 0 -> red
        // {0, 1, 0},   // material 1 -> green
        // {0, 0, 1},   // material 2 -> blue
        // {1, 1, 0},   // material 3 -> yellow
        // {1, 0, 1},   // material 4 -> magenta
        // {0, 1, 1}    // material 5 -> cyan
        // };
        // int mid = std::max(0, T.material);
        // float3 color = matColors[mid % 6];

        float3 irradiance = shade_with_sh(sh, h.mesh, i0, i1, i2, b0, b1, b2);
        float3 albedo = {0.8f, 0.8f, 0.8f};
        if (T.material >= 0 && T.material < (int)materials.Kd_linear.size()) {
          albedo = materials.Kd_linear[T.material];
        }
        float3 color = irradiance * albedo;

        // if (T.material == 3 && x >= W/2 && y >= H/2) {
        //     std::cout << "irr = " << irradiance.x << "," << irradiance.y <<
        //     "," << irradiance.z
        //               << " alb = " << albedo.x << "," << albedo.y << "," <<
        //               albedo.z
        //               << " col = " << color.x << "," << color.y << "," <<
        //               color.z << "\n";
        // }

        write(x, y, color);
      } else {
        write(x, y, float3{0.05f, 0.05f, 0.08f});
      }
    }
  }
}
