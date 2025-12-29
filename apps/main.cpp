#include <iostream>
#include <vector>

#include "Win32Display.h"
#include "bvh.h"
#include "load_obj_fast.h"
#include "renderer.h"
#include "sh_lighting.h"
#include "tracy/Tracy.hpp"

std::string cup_obj = "C:/Users/vivek/CLionProjects/raytracer2/copa.obj";
std::string blender_obj =
    "C:/Users/vivek/CLionProjects/raytracer2/robin-green-sh-scene.obj";
std::string blender2_obj =
    "C:/Users/vivek/CLionProjects/raytracer2/robin-green-sh-scene2.obj";
std::string cornell_box_obj = "C:/Users/vivek/CLionProjects/raytracer2/cornell-box-cycles.obj";
static inline void frameCameraToAABB(Camera &cam, const AABB &bbox,
                                     float fovy_rads, float aspect,
                                     float3 view_dir = {0, 0, -1},
                                     float3 up_hint = {0, 1, 0},
                                     float margin = 1.2f) {
  ZoneScopedN("frameCameraToAABB");
  float r = __max(0.001f, bbox.radius());
  float fovx = 2.0f * atanf(tanf(fovy_rads * 0.5f) * aspect);
  float d_v = r / tanf(fovy_rads * 0.5f);
  float d_h = r / tanf(fovx * 0.5f);
  float d = margin * __max(d_v, d_h);

  float3 center = bbox.center();
  float3 f = normalize(view_dir);
  float3 cam_pos = center - d * f;

  float3 rvec = normalize(cross(f, up_hint));
  if (length(rvec) == 0.0f)
    rvec = {1, 0, 0};
  float3 uvec = cross(rvec, f);

  cam.pos = cam_pos;
  cam.forward = f;
  cam.dir = f;
  cam.right = rvec;
  cam.up = uvec;

  cam.fovy = fovy_rads;
  cam.aspect = aspect;
  cam.near_plane = __max(0.001f, d - 1.5f * r);
  cam.far_plane = d + 1.5f * r;
}

int wmain() {
  ZoneScopedN("Total run");

  // const int W = 3840, H = 2160;
  // const int W = 2560, H = 1440;
  const int W = 1920, H = 1080;
  Camera cam{};
  AABB sceneBounds;

  Win32Display disp(W, H, L"rt + fast_obj");
  if (!disp.create())
    return -1;

  std::vector<TriangleMesh> scene;
  Materials mats{};
  {
    ZoneScopedN("Load");
    if (!load_triangle_meshes_fastobj(cornell_box_obj, scene, mats, sceneBounds))
      return -2;
  }

  BVH bvh;
  {
    ZoneScopedN("Build Scene");
    bvh.buildFromScene(scene);
  }

  SHContext sh;
  {
    ZoneScopedN("SH Pre-compute");
    init_sh_context(sh, 2);
    // bake_diffuse_unshadowed_transfers(scene, mats, sh);
    bake_diffuse_shadowed_transfers(scene, mats, bvh, sh);
    bake_interreflected_transfers(scene, mats, bvh, sh, 10);
  }

  size_t tris = 0, verts = 0;
  for (auto &M : scene) {
    tris += M.tris.size();
    verts += M.positions.size();
  }
  std::cout << "scene: " << scene.size() << " meshes, " << tris << " tris, "
            << verts << " verts\n";

  {
    ZoneScopedN("Camera properties");
    frameCameraToAABB(cam, sceneBounds, radians(50.0f), float(W) / float(H),
                      float3{0, 0, -0.9f}, float3{0, 1, 0}, 1.4f);
  }

  while (disp.process_messages()) {
    // ZoneScopedN("Render");
    unsigned char *bgr = disp.get_pixel_buffer();
    // printf("got pixel buffer...about to render frame");
    {
      ZoneScopedN("RenderImage");
      render_bgr24(scene, bvh, mats, sh, cam, W, H, bgr);
      // render_bgr24(scene, bvh, mats, cam, W, H, bgr);
    }
    // count += 1;
    disp.update_window();
    // disp.process_messages();
  }

  return 0;
}
