//
// Created by Vivek Ramadhar on 12/14/25.
//
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <vector>
#include <string>
#include <fstream>
#include <thread>

#include "base_win32.h"
#include "sh_lighting.h"
#include "load_obj_fast.h"
#include "orbit_camera.h"

namespace py = pybind11;

void shrender(std::string obj_path, std::vector<float> cam_eye, std::vector<float> cam_target,
              int numBounces,
              float li, float fov_deg, int width, int height) {
    StaticWindow disp(width, height, L"SH Renderer (Python Controlled)");
    if (!disp.create(disp.ClassName(), WS_OVERLAPPEDWINDOW | WS_VISIBLE, WS_EX_CLIENTEDGE,
    CW_USEDEFAULT, CW_USEDEFAULT, width, height)) {
        throw std::runtime_error("Failed to create window");
    }
    disp.init_pixel_buffer();
    disp.init_dib();
    disp.show();

    Camera cam;
    cam.aspect = (float)width / (float)height;

    float3 eye = { cam_eye[0], cam_eye[1], cam_eye[2] };
    float3 target = { cam_target[0], cam_target[1], cam_target[2] };

    cam.pos = eye;
    cam.forward = normalize(target-eye);
    cam.right = normalize(cross(cam.forward, float3{0, 1, 0}));
    cam.up = normalize(cross(cam.right, cam.forward));
    cam.fovy = fov_deg;

    AABB sceneBounds;
    std::vector<TriangleMesh> scene;
    Materials mats{};
    if (!load_triangle_meshes_fastobj(obj_path, scene, mats, sceneBounds)){
        throw std::runtime_error("Could not load OBJ: " + obj_path);
    }

    BVH bvh;
    bvh.buildFromScene(scene);

    SHContext sh;

    init_sh_context(sh, li);
    bake_diffuse_shadowed_transfers(scene, mats, bvh, sh);
    bake_interreflected_transfers(scene, mats, bvh, sh, numBounces);

    render_bgr24_mt(scene, bvh, mats, sh, cam, width, height, disp.get_pixel_buffer(), std::thread::hardware_concurrency()-2);

    disp.update_window();

    while (disp.show()) {

    }


}

void sh_orbit_render(std::string obj_path, int numBounces, float li, int width, int height) {
    OrbitWindow disp(width, height, L"SH Renderer (Python Controlled)");
    if (!disp.create(disp.ClassName(), WS_OVERLAPPEDWINDOW | WS_VISIBLE, WS_EX_CLIENTEDGE,
    CW_USEDEFAULT, CW_USEDEFAULT, width, height)) {
        throw std::runtime_error("Failed to create window");
    }
    disp.init_pixel_buffer();
    disp.init_dib();
    disp.show();

    AABB sceneBounds;
    std::vector<TriangleMesh> scene;
    Materials mats{};
    if (!load_triangle_meshes_fastobj(obj_path, scene, mats, sceneBounds)){
        throw std::runtime_error("Could not load OBJ: " + obj_path);
    }

    BVH bvh;
    bvh.buildFromScene(scene);

    SHContext sh;

    init_sh_context(sh, li);
    bake_diffuse_shadowed_transfers(scene, mats, bvh, sh);
    bake_interreflected_transfers(scene, mats, bvh, sh, numBounces);

    Camera cam;
    OrbitCamera orbcam;
    frame_scene(orbcam, sceneBounds, 40.0f);
    cam.aspect = (float)width/(float)height;
    orbit_look_at(orbcam, cam);

    int num_threads = max(1, std::thread::hardware_concurrency()-2);

    render_bgr24_mt(scene, bvh, mats, sh, cam, width, height, disp.get_pixel_buffer(), num_threads);

    disp.update_window();

    while (disp.show()) {
        int dx, dy, scroll, pan_dx, pan_dy;
        disp.consume_input(dx, dy, scroll, pan_dx, pan_dy);
        disp.clear_dirty();

        if (dx != 0 || dy != 0) orbit(orbcam, dx, dy);
        if (scroll != 0) zoom(orbcam, scroll);
        if (pan_dx != 0|| pan_dy != 0) pan(orbcam, cam, 0.02f, pan_dx, pan_dy);

        orbit_look_at(orbcam, cam);
        render_bgr24_mt(scene, bvh, mats, sh, cam, disp.m_width, disp.m_height,
                       disp.get_pixel_buffer(), num_threads);
        disp.update_window();
    }

}


PYBIND11_MODULE(sh_engine, m) {
    m.doc() = "Native C++ SH Renderer";
    m.def("render", &shrender, "Renders an OBJ file",
        py::arg("obj_path"),
        py::arg("eye"),
        py::arg("target"),
        py::arg("numBounces") = 10,
        py::arg("li") = 1.0f,
        py::arg("fov") = 45.0f,
        py::arg("width") = 800,
        py::arg("height") = 600
        );

    m.def("orbit_render", &sh_orbit_render, "Opens an OBJ in an interactive viewer with an orbit camera",
        py::call_guard<py::gil_scoped_release>(),
        py::arg("obj_path"),
        py::arg("bounces") = 5,
        py::arg("lightIntensity") =  1.0f,
        py::arg("width") = 800,
        py::arg("height") = 600
    );
}