//
// Created by vivek on 11/5/2025.
//
#include "sh_lighting.h"
#include <algorithm>
#include <cmath>
#include <iostream>

#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"
#endif
#include "nittygritty.h"

static constexpr double kPi = 3.14159265358979323846;

static double sky_red(double theta, double phi) {
  return (theta < 0.5 * kPi) ? 1.0 : 0.15;
}

static double sky_green(double theta, double phi) {
  return (theta < 0.5 * kPi) ? 0.95 : 0.13;
}

static double sky_blue(double theta, double phi) {
  return (theta < 0.5 * kPi) ? 0.7 : 0.12;
}

static float3 Ldir = normalize(float3{0.6f, 1.0f, 0.4f});

static double sky_dir(double theta, double phi) {
  double sinT = std::sin(theta);
  float3 w{(float)(sinT * std::cos(phi)), (float)(sinT * std::sin(phi)),
           (float)std::cos(theta)};
  double ndotL = dot(w, Ldir);
  return ndotL > 0.0 ? ndotL : 0.0;
}

void init_sh_context(SHContext &ctx, const double lightIntensity) {
  #ifdef TRACY_ENABLE
    ZoneScopedN("init_sh_context");
  #endif
  ctx.samples.resize(SH_SAMPLES);
  SH_setup_spherical_samples(ctx.samples, SH_SQRT_SAMPLES, SH_BANDS);
  std::vector<double> tmp(SH_COEFFS);

  // Red
  std::fill(tmp.begin(), tmp.end(), 0.0);
  SH_project_polar_function_vec(sky_red, ctx.samples, tmp, SH_SAMPLES,
                                SH_COEFFS);
  for (int i = 0; i < SH_COEFFS; ++i) {
    ctx.lightCoeffs[i] = tmp[i];
  }

  // Green
  std::fill(tmp.begin(), tmp.end(), 0.0);
  SH_project_polar_function_vec(sky_green, ctx.samples, tmp, SH_SAMPLES,
                                SH_COEFFS);
  for (int i = 0; i < SH_COEFFS; ++i) {
    ctx.lightCoeffs[SH_COEFFS + i] = tmp[i];
  }

  // Blue
  std::fill(tmp.begin(), tmp.end(), 0.0);
  SH_project_polar_function_vec(sky_blue, ctx.samples, tmp, SH_SAMPLES,
                                SH_COEFFS);
  for (int i = 0; i < SH_COEFFS; ++i) {
    ctx.lightCoeffs[2 * SH_COEFFS + i] = tmp[i];
  }

  // double lightIntensity = 2.0;
  for (double &c : ctx.lightCoeffs) {
    c *= lightIntensity;
  }
}

static void compute_diffuse_unshadowed_transfer(const SHContext &ctx,
                                                const float3 &normal,
                                                SHTransfer &outTransfer) {
  for (double &c : outTransfer.coeff)
    c = 0.0;

  const int n_samples = static_cast<int>(ctx.samples.size());
  const int n_coeff = SH_COEFFS;

  float3 N = normalize(normal);
  // float3 A  = albedo;

  for (int i = 0; i < n_samples; ++i) {
    const SHSample_vec &s = ctx.samples[i];

    double H = dot(s.vec, N);
    if (H <= 0.0)
      continue;
    for (int j = 0; j < n_coeff; ++j) {
      double value = H * s.coeff[j];

      outTransfer.coeff[j] += value;
      outTransfer.coeff[j + n_coeff] += value;
      outTransfer.coeff[j + 2 * n_coeff] += value;
    }
  }

  const double area = 4.0 * kPi;
  const double factor = area / double(n_samples);

  for (double &c : outTransfer.coeff)
    c *= factor;
}

void bake_diffuse_unshadowed_transfers(const std::vector<TriangleMesh> &scene,
                                       const Materials &materials,
                                       SHContext &ctx) {
  #ifdef TRACY_ENABLE
    ZoneScopedN("bake_diffuse_unshadowed_transfers");
  #endif
  ctx.meshTransfers.clear();
  ctx.meshTransfers.resize(scene.size());

  for (size_t mi = 0; mi < scene.size(); ++mi) {
    const TriangleMesh &M = scene[mi];
    const int nVerts = static_cast<int>(M.positions.size());
    ctx.meshTransfers[mi].resize(nVerts);

    std::vector<float3> vertAlbedo(nVerts, float3{0, 0, 0});
    std::vector<int> vertCount(nVerts, 0);
    std::vector<float3> vertNormal(nVerts, float3{0, 0, 0});

    for (const Triangle &T : M.tris) {
      float3 triAlbedo = float3{0.8f, 0.8f, 0.8f};
      if (T.material >= 0 && T.material < (int)materials.Kd_linear.size()) {
        triAlbedo = materials.Kd_linear[T.material];
      }
      const float3 &p0 = M.positions[T.i0];
      const float3 &p1 = M.positions[T.i1];
      const float3 &p2 = M.positions[T.i2];

      float3 faceN = cross(p1 - p0, p2 - p0);
      float len2 = dot(faceN, faceN);
      if (len2 > 1e-12) {
        faceN = faceN / sqrtf(len2);
      } else {
        faceN = float3{0, 1, 0};
      }

      vertAlbedo[T.i0] += triAlbedo;
      vertCount[T.i0]++;
      vertNormal[T.i0] += faceN;
      vertAlbedo[T.i1] += triAlbedo;
      vertCount[T.i1]++;
      vertNormal[T.i1] += faceN;
      vertAlbedo[T.i2] += triAlbedo;
      vertCount[T.i2]++;
      vertNormal[T.i2] += faceN;
    }

    for (int v = 0; v < nVerts; ++v) {
      float3 normal = vertNormal[v];
      float len2 = dot(normal, normal);
      if (!std::isfinite(len2) || len2 <= 1e-12) {
        normal = float3{0, 1, 0};
      } else {
        normal = normalize(normal);
      }
      compute_diffuse_unshadowed_transfer(ctx, normal,
                                          ctx.meshTransfers[mi][v]);
    }
  }
}

void compute_diffuse_shadowed_transfer(const SHContext &ctx, const BVH &bvh,
                                       const float3 &position,
                                       const float3 &normal,
                                       SHTransfer &outTransfer) {
  #ifdef TRACY_ENABLE
    ZoneScopedN("compute_diffuse_shadowed_transfer");
  #endif
  for (double &c : outTransfer.coeff)
    c = 0.0;

  const int n_samples = static_cast<int>(ctx.samples.size());
  const int n_coeff = SH_COEFFS;

  float3 N = normalize(normal);

  const float eps = 1e-3f;

  for (int i = 0; i < n_samples; ++i) {
    const SHSample_vec &s = ctx.samples[i];

    double H = dot(s.vec, N);
    if (H <= 0.0)
      continue;

    Ray ray;
    ray.o = position + N * eps;
    ray.d = s.vec;

    Hit h;
    {
      // Fast BVH made this whole function really fast!
      #ifdef TRACY_ENABLE
        ZoneScopedN("bvh.intersect - diffuse_shadowed_transfer");
      #endif
      if (bvh.intersect(ray, h)) {
        continue; // V = 0
      }
    }

    // Visible -> accumulate SH basis * cosine
    for (int j = 0; j < n_coeff; ++j) {
      double value = H * s.coeff[j];

      outTransfer.coeff[j] += value;
      outTransfer.coeff[j + n_coeff] += value;
      outTransfer.coeff[j + 2 * n_coeff] += value;
    }
  }

  // Monte-Carlo weight
  const double area = 4.0 * kPi;
  const double factor = area / double(n_samples);

  for (double &c : outTransfer.coeff)
    c *= factor;
}

void bake_diffuse_shadowed_transfers(const std::vector<TriangleMesh> &scene,
                                     const Materials &materials, const BVH &bvh,
                                     SHContext &ctx) {
  {
    #ifdef TRACY_ENABLE
      ZoneScopedN("bake_diffuse_shadowed_transfers");
    #endif

    ctx.meshTransfers.clear();
    ctx.meshTransfers.resize(scene.size());

    for (size_t mi = 0; mi < scene.size(); ++mi) {
      const TriangleMesh &M = scene[mi];
      const int nVerts = static_cast<int>(M.positions.size());
      ctx.meshTransfers[mi].resize(nVerts);

      std::vector<float3> vertAlbedo(nVerts, float3{0, 0, 0});
      std::vector<int> vertCount(nVerts, 0);
      std::vector<float3> vertNormal(nVerts, float3{0, 0, 0});

      for (const Triangle &T : M.tris) {
        float3 triAlbedo = float3{0.8f, 0.8f, 0.8f};
        if (T.material >= 0 && T.material < (int)materials.Kd_linear.size()) {
          triAlbedo = materials.Kd_linear[T.material];
        }

        const float3 &p0 = M.positions[T.i0];
        const float3 &p1 = M.positions[T.i1];
        const float3 &p2 = M.positions[T.i2];

        float3 faceN = cross(p1 - p0, p2 - p0);
        float len2 = dot(faceN, faceN);
        if (len2 > 1e-12f) {
          faceN = faceN / sqrtf(len2);
        } else {
          faceN = float3{0, 1, 0};
        }

        vertAlbedo[T.i0] += triAlbedo;
        vertCount[T.i0]++;
        vertNormal[T.i0] += faceN;
        vertAlbedo[T.i1] += triAlbedo;
        vertCount[T.i1]++;
        vertNormal[T.i1] += faceN;
        vertAlbedo[T.i2] += triAlbedo;
        vertCount[T.i2]++;
        vertNormal[T.i2] += faceN;
      }

      for (int v = 0; v < nVerts; ++v) {
        float3 normal = vertNormal[v];
        float len2 = dot(normal, normal);
        if (!std::isfinite(len2) || len2 <= 1e-12) {
          normal = float3{0, 1, 0};
        } else {
          normal = normalize(normal);
        }

        const float3 &pos = M.positions[v];

        compute_diffuse_shadowed_transfer(ctx, bvh, pos, normal,
                                          ctx.meshTransfers[mi][v]);
      }
    }
  }
}

void compute_self_transfer_bounce(
    const SHContext &ctx, const BVH &bvh,
    const std::vector<TriangleMesh> &scene,
    const std::vector<std::vector<SHTransfer>> &prevTransfers,
    const Materials &materials, const float3 &position, const float3 &normal,
    SHTransfer &outTransfer) {

  for (double &c : outTransfer.coeff)
    c = 0.0;

  const int n_samples = static_cast<int>(ctx.samples.size());
  const int n_coeff = SH_COEFFS;
  const float eps = 1e-3;

  float3 N = normalize(normal);

  for (int i = 0; i < n_samples; ++i) {
    const SHSample_vec &s = ctx.samples[i];

    double H = dot(s.vec, N);
    if (H <= 0.0)
      continue;

    Ray ray;
    ray.o = position + N * eps;
    ray.d = s.vec;

    Hit h;
    if (bvh.intersect(ray, h)) {
      const TriangleMesh &hitMesh = scene[h.mesh];
      const Triangle &hitTri = hitMesh.tris[h.tri];

      float w = 1.0f - h.u - h.v;
      const std::vector<SHTransfer> &hitTransfers = prevTransfers[h.mesh];

      const SHTransfer &t0 = hitTransfers[hitTri.i0];
      const SHTransfer &t1 = hitTransfers[hitTri.i1];
      const SHTransfer &t2 = hitTransfers[hitTri.i2];

      float3 hitAlbedo = float3{0.8f, 0.8f, 0.8f};
      if (hitTri.material >= 0 &&
          hitTri.material < (int)materials.Kd_linear.size()) {
        hitAlbedo = materials.Kd_linear[hitTri.material];
      }

      for (int k = 0; k < 3 * SH_COEFFS; ++k) {
        double sh_val = w * t0.coeff[k] + h.u * t1.coeff[k] + h.v * t2.coeff[k];
        double colorMod = 0;

        if (k < n_coeff)
          colorMod = hitAlbedo.x;
        else if (k < 2 * n_coeff)
          colorMod = hitAlbedo.y;
        else
          colorMod = hitAlbedo.z;

        outTransfer.coeff[k] += sh_val * colorMod * H;
      }
    }
  }

  const double factor = (4.0 * kPi) / double(n_samples);
  for (double &c : outTransfer.coeff) {
    c *= factor;
  }

}

void bake_interreflected_transfers(const std::vector<TriangleMesh> &scene,
                                   const Materials &materials, const BVH &bvh,
                                   SHContext &ctx, int numBounces) {
  #ifdef TRACY_ENABLE
    ZoneScopedN("bake_interreflected_transfers");
  #endif
  std::vector<std::vector<SHTransfer>> prevBounce = ctx.meshTransfers;
  for (int bounce = 0; bounce < numBounces; ++bounce) {
    std::vector<std::vector<SHTransfer>> nextBounce;
    nextBounce.resize(scene.size());
    for (size_t i = 0; i < scene.size(); ++i) {
      nextBounce[i].resize(scene[i].positions.size());
    }

    for (size_t mi = 0; mi < scene.size(); ++mi) {
      const TriangleMesh &M = scene[mi];
      const int nVerts = static_cast<int>(M.positions.size());

      std::vector<float3> vertNormal(nVerts, float3{0, 0, 0});
      for (const Triangle &T : M.tris) {
        const float3 &p0 = M.positions[T.i0];
        const float3 &p1 = M.positions[T.i1];
        const float3 &p2 = M.positions[T.i2];
        float3 faceN = cross(p1 - p0, p2 - p0);
        float len2 = dot(faceN, faceN);
        if (len2 > 1e-12)
          faceN = faceN / sqrtf(len2);
        else
          faceN = float3{0, 1, 0};
        vertNormal[T.i0] += faceN;
        vertNormal[T.i1] += faceN;
        vertNormal[T.i2] += faceN;
      }

      for (int v = 0; v < nVerts; ++v) {
        float3 normal = vertNormal[v];
        float len2 = dot(normal, normal);
        if (std::isfinite(len2) || len2 <= 1e-12)
          normal = float3{0, 1, 0};
        else
          normal = normalize(normal);
        const float3 &pos = M.positions[v];
        compute_self_transfer_bounce(ctx, bvh, scene, prevBounce, materials,
                                     pos, normal, nextBounce[mi][v]);
      }
    }

    for (size_t m = 0; m < scene.size(); ++m) {
      for (size_t v = 0; v < scene[m].positions.size(); ++v) {
        for (int k = 0; k < 3 * SH_COEFFS; ++k) {
          ctx.meshTransfers[m][v].coeff[k] += nextBounce[m][v].coeff[k];
        }
      }
    }

    prevBounce = nextBounce;
  }
}

static float3 evaluate_sh_lighting(const SHContext &ctx, const SHTransfer &T) {
  float3 c{0.f, 0.f, 0.f};

  for (int i = 0; i < SH_COEFFS; ++i) {
    c.x += static_cast<float>(T.coeff[i] * ctx.lightCoeffs[i]);
    c.y += static_cast<float>(T.coeff[i + SH_COEFFS] *
                              ctx.lightCoeffs[i + SH_COEFFS]);
    c.z += static_cast<float>(T.coeff[i + 2 * SH_COEFFS] *
                              ctx.lightCoeffs[i + 2 * SH_COEFFS]);
  }
  return c;
}

float3 shade_with_sh(const SHContext &ctx, int meshIndex, int i0, int i1,
                     int i2, float b0, float b1, float b2) {
  const auto &transfers = ctx.meshTransfers[meshIndex];
  const SHTransfer &t0 = transfers[i0];
  const SHTransfer &t1 = transfers[i1];
  const SHTransfer &t2 = transfers[i2];

  SHTransfer T_interp;
  for (int k = 0; k < 3 * SH_COEFFS; ++k) {
    T_interp.coeff[k] = b0 * t0.coeff[k] + b1 * t1.coeff[k] + b2 * t2.coeff[k];
  }

  return evaluate_sh_lighting(ctx, T_interp);
}
