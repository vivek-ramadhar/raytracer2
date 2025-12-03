//
// Created by vivek on 11/5/2025.
//
#include <array>
#include <vector>

#include "bvh.h"
#include "load_obj_fast.h"
#include "mesh.h"
#include "nittygritty.h"
#include "vmath.h"

#ifndef SH_LIGHTING_H
#define SH_LIGHTING_H

constexpr int SH_BANDS = 7;
constexpr int SH_COEFFS = SH_BANDS * SH_BANDS;
constexpr int SH_SQRT_SAMPLES = 16;
constexpr int SH_SAMPLES = SH_SQRT_SAMPLES * SH_SQRT_SAMPLES;

struct SHTransfer {
  std::array<double, 3 * SH_COEFFS> coeff{};
};

struct SHContext {
  std::vector<SHSample_vec> samples;
  std::array<double, 3 * SH_COEFFS> lightCoeffs{};
  std::vector<std::vector<SHTransfer>> meshTransfers;
  float totalEmitterArea = 0.0f;
};

void init_sh_context(SHContext &ctx, const double lightIntensity=1.5);

static void compute_diffuse_unshadowed_transfer(const SHContext &ctx,
                                                const float3 &normal,
                                                SHTransfer &outTransfer);

void bake_diffuse_unshadowed_transfers(const std::vector<TriangleMesh> &scene,
                                       const Materials &materials,
                                       SHContext &ctx);

static void compute_diffuse_shadowed_transfer(const SHContext &ctx,
                                              const BVH &bvh,
                                              const float3 &position,
                                              const float3 &normal,
                                              SHTransfer &outTransfer);

void bake_diffuse_shadowed_transfers(const std::vector<TriangleMesh> &scene,
                                     const Materials &materials, const BVH &bvh,
                                     SHContext &ctx);

static void compute_self_transfer_bounce(
    const SHContext &ctx, const BVH &bvh,
    const std::vector<TriangleMesh> &scene,
    const std::vector<std::vector<SHTransfer>> &prevTransfers,
    const Materials &materials, const float3 &position, const float3 &normal,
    SHTransfer &outTransfer);

void bake_interreflected_transfers(const std::vector<TriangleMesh> &scene,
                                   const Materials &materials, const BVH &bvh,
                                   SHContext &ctx, int numBounces);

float3 shade_with_sh(const SHContext &ctx, int meshIndex, int i0, int i1,
                     int i2, float b0, float b1, float b2);

#endif // SH_LIGHTING_H
