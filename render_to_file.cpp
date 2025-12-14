//
// Created by Vivek Ramadhar on 12/13/25.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "bvh.h"
#include "load_obj_fast.h"
#include "renderer.h"
#include "sh_lighting.h"
#include "tracy/Tracy.hpp"

void SaveAsPPM(const std::string& filename, const uchar* bgr, int width, int height) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::clog << "Failed to open file for writing: " << filename << "\n";
        return;
    }

    file << "P3\n" << width << " " << height << "\n255\n";
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int index = (j * width + i) * 3;
            uint8_t b = bgr[index + 0]; // BGR order for Windows
            uint8_t g = bgr[index + 1];
            uint8_t r = bgr[index + 2];
            file << (int)r << " " << (int)g << " " << (int)b << "\n";
        }
    }

    file.close();
    std::clog << "Saved render as: " << filename << "\n";
}

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

int main(int argc, char** argv) {
    std::string file = argv[1];
    const int W = 1920, H = 1080;
    Camera cam;
    AABB sceneBounds;
    std::vector<TriangleMesh> scene;
    Materials mats;
    if (!load_triangle_meshes_fastobj(file, scene, mats, sceneBounds)) {
        return -2;
    }
    BVH bvh;
    bvh.buildFromScene(scene);
    SHContext sh;
    init_sh_context(sh, 1.5);
    bake_diffuse_shadowed_transfers(scene, mats, bvh, sh);
    bake_interreflected_transfers(scene, mats, bvh, sh, 10);
    size_t tris = 0, verts = 0;
    for (auto& M : scene) {
        tris += M.tris.size();
        verts += M.positions.size();
    }
    std::cout << "scene: " << scene.size() << " meshes, " << tris << " tris, "
                << verts << " verts\n";

    frameCameraToAABB(cam, sceneBounds, radians(50.0f), float(W)/float(H),
                      float3{0.5, -0.5, 0.5f}, float3{0, 1, 0}, 1.4f);

    uchar pixel_buffer[W*H*3];
    render_bgr24(scene, bvh, mats, sh, cam, W, H, pixel_buffer);
    SaveAsPPM("output.ppm", pixel_buffer, W, H);
    return 0;
}
