//
// Created by vivek on 2/6/2026.
//
#include "base_win32.h"
#include "bvh.h"
#include "load_obj_fast.h"
#include "orbit_camera.h"
#include <iostream>
#include <thread>
#include <vector>
#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

std::string cup_obj = "C:/Users/vivek/CLionProjects/raytracer2/copa.obj";
std::string blender_obj =
    "C:/Users/vivek/CLionProjects/raytracer2/robin-green-sh-scene.obj";
std::string blender2_obj =
    R"(C:\Users\vivek\CLionProjects\raytracer2\assets\robin-green-sh-scene2.obj)";
// View settings for this OBJ
// float3 view_dir = float3(1.0f, -0.2f, 1.0f);
// FrameScene(cam, sceneBounds, view_dir, 45.0f);
std::string cornell_box_obj = R"(C:\Users\vivek\CLionProjects\raytracer2\scripts\cornell_box.obj)";

int wmain(int argc, wchar_t* argv[]) {
#ifdef TRACY_ENABLE
    ZoneScopedN("Total Run");
#endif

    int W = 1280, H = 720;
    OrbitWindow disp(W, H, L"Win32 Rewrite");
    if (!disp.create(disp.ClassName(), WS_OVERLAPPEDWINDOW, WS_EX_CLIENTEDGE, CW_USEDEFAULT, CW_USEDEFAULT, W, H)) {
        std::cout << "Failed to create Win32 Window" << "\n";
        return -1;
    }
    disp.init_pixel_buffer();
    disp.init_dib();
    disp.show();

    // cam.pos = {0.0f, 1.0f, 3.0f};  // Move back 3 units
    // cam.forward = {0.0f, 0.0f, -1.0f}; // Look forward (usually -Z)
    // cam.up = {0.0f, 1.0f, 0.0f};
    // cam.right = {1.0f, 0.0f, 0.0f}; // Cross(forward, up)
    // cam.fovy = 45.0f;
    AABB sceneBounds;
    std::vector<TriangleMesh> scene;
    Materials mats{};

    {
#ifdef TRACY_ENABLE
        ZoneScopedN("Load Obj");
#endif
        std::string cornell_box_embree = "C:/Users/vivek/CLionProjects/raytracer2/assets/cornell_embree_tri_divs.obj";
        std::string bmw = "C:/Users/vivek/CLionProjects/raytracer2/assets/bmw_tri.obj";
        if (!load_triangle_meshes_fastobj(bmw, scene, mats, sceneBounds)) {
            std::cout << "Failed to load scene " << bmw << "\n";
            return -2;
        }
    }

    BVH bvh;
    {
#ifdef TRACY_ENABLE
        ZoneScopedN("Build Scene");
#endif
        bvh.buildFromScene(scene);
    }

    std::cout << "Scene Bounds: " << sceneBounds.minv.x << " " << sceneBounds.minv.y << " " << sceneBounds.minv.z
    << " to " << sceneBounds.maxv.x << " " << sceneBounds.maxv.y << " " << sceneBounds.maxv.z;

    SHContext sh;
    {
#ifdef TRACY_ENABL1
        ZoneScopedN("SH Pre-compte");
#endif
        init_sh_context(sh, 2);
        bake_diffuse_shadowed_transfers(scene, mats, bvh, sh);
        bake_interreflected_transfers(scene, mats, bvh, sh, 2);
    }

    size_t tris = 0, verts = 0;
    for (auto &M : scene) {
        tris += M.tris.size();
        verts += M.positions.size();
    }
    std::cout << "scene: " << scene.size() << " meshes, " << tris << " tris, "
            << verts << " verts\n";


    Camera cam;
    OrbitCamera orbcam;
    frame_scene(orbcam, sceneBounds, 40.0f);
    cam.aspect = (float)W/(float)H;
    orbit_look_at(orbcam, cam);

    int num_threads = max(1, std::thread::hardware_concurrency()-2);

    {
#ifdef TRACY_ENABLE
        ZoneScopedN("Render Image");
#endif
        render_bgr24_mt(scene, bvh, mats, sh, cam, disp.m_width, disp.m_height,
                    disp.get_pixel_buffer(), num_threads);
        // render_bgr24(scene, bvh, mats, sh, cam, W, H, disp.get_pixel_buffer());
    }
    disp.update_window();
    while (disp.show()) {
        // if (!disp.is_camera_dirty()) {
        //     Sleep(1);
        //     continue;
        // }

        {
#ifdef TRACY_ENABLE
            ZoneScopedN("process input");
#endif
            int dx, dy, scroll, pan_dx, pan_dy;
            disp.consume_input(dx, dy, scroll, pan_dx, pan_dy);
            disp.clear_dirty();

            if (dx != 0 || dy != 0) orbit(orbcam, dx, dy);
            if (scroll != 0) zoom(orbcam, scroll);
            if (pan_dx != 0|| pan_dy != 0) pan(orbcam, cam, 0.02f, pan_dx, pan_dy);
        }

        {

#ifdef TRACY_ENABLE
            ZoneScopedN("orbit_look_at");
#endif
            orbit_look_at(orbcam, cam);
        }
        render_bgr24_mt(scene, bvh, mats, sh, cam, disp.m_width, disp.m_height,
                       disp.get_pixel_buffer(), num_threads);
        disp.update_window();

    }
}
