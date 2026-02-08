//
// Created by vivek on 2/7/2026.
//

#include "base_win32.h"
#include <windows.h>
#include <windowsx.h>
#include "renderer.h"

#ifndef RAYTRACER2_ORBIT_CAMERA_H
#define RAYTRACER2_ORBIT_CAMERA_H
struct OrbitCamera {
    float yaw = 0.0f, pitch = 0.0f, radius = 0.0f;
    float3 target = {0, 0, 0};

    float min_radius = 0.1f;
    float max_radius = 10000.0f;
    float fov = 45.0f;
};

void orbit_look_at(OrbitCamera& orbcam, Camera& cam) {
    float3 up_hint = float3{0, 1, 0};
    float3 eye = orbcam.target + orbcam.radius * float3(cosf(orbcam.pitch)*sinf(orbcam.yaw), sinf(orbcam.pitch),
        cosf(orbcam.pitch)*cosf(orbcam.yaw));
    cam.pos = eye;
    cam.forward = normalize(orbcam.target - eye);

    if (fabs(dot(cam.forward, up_hint)) > 0.99f) {
        up_hint = {0, 0, 1};
    }

    cam.right = normalize(cross(cam.forward, up_hint));
    cam.up = normalize(cross(cam.right, cam.forward));
    cam.fovy = orbcam.fov;
}

void orbit(OrbitCamera& orbcam, float dx, float dy, float sensitivity = 0.005f) {
    orbcam.yaw -= dx * sensitivity;
    orbcam.pitch += dy * sensitivity;
    orbcam.pitch = clamp(orbcam.pitch, -1.5f, 1.5f);
}

void zoom(OrbitCamera& orbcam, float scroll_delta) {
    float ticks = scroll_delta / 120.0f;
    float zfactor = powf(0.9f, ticks);;
    orbcam.radius *= zfactor;
    orbcam.radius = clamp(orbcam.radius, orbcam.min_radius, orbcam.max_radius);
}

void pan(OrbitCamera& orbcam, Camera cam, float pan_sensitivity, float dx, float dy) {
    float scale = orbcam.radius * pan_sensitivity;
    orbcam.target -= cam.right * (dx * scale);
    orbcam.target += cam.up * (dy * scale);
}

void frame_scene(OrbitCamera& orbcam, const AABB& bounds, float fov_deg = 45.0f) {
    orbcam.target = bounds.center();
    orbcam.fov = fov_deg;

    float scene_radius = length(bounds.maxv - bounds.minv) * 0.5f;
    float dist = scene_radius / sinf(fov_deg * pi/360.0f);
    orbcam.radius = dist * 1.2f;

    orbcam.yaw = 0.0f;
    orbcam.pitch = 0.15f;
}

class OrbitWindow : public StaticWindow {
public:
    // reuse constructor
    using StaticWindow::StaticWindow;

    bool m_lmb_down = false, m_mmb_down = false;
    int m_last_mouse_x = 0, m_last_mouse_y = 0, m_mouse_dx = 0, m_mouse_dy = 0;
    int m_scroll_delta = 0;
    int m_pan_dx = 0,  m_pan_dy = 0;
    bool m_camera_dirty = false;

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) override {
        switch (msg) {
            case WM_LBUTTONDOWN: {
                m_lmb_down = true;
                m_last_mouse_x = GET_X_LPARAM(lParam);
                m_last_mouse_y = GET_Y_LPARAM(lParam);
                SetCapture(m_hwnd);
                return 0;
            }

            case WM_LBUTTONUP: {
                m_lmb_down = false;
                ReleaseCapture();
                return 0;
            }

            case WM_MBUTTONDOWN: {
                m_mmb_down = true;
                m_last_mouse_x = GET_X_LPARAM(lParam);
                m_last_mouse_y = GET_Y_LPARAM(lParam);
                SetCapture(m_hwnd);
                return 0;
            }

            case WM_MBUTTONUP: {
                m_mmb_down = false;
                ReleaseCapture();
                return 0;
            }

            case WM_MOUSEMOVE: {
                int mx = GET_X_LPARAM(lParam);
                int my = GET_Y_LPARAM(lParam);
                if (m_lmb_down) {
                    m_mouse_dx += (mx - m_last_mouse_x);
                    m_mouse_dy += (my - m_last_mouse_y);
                    m_camera_dirty = true;
                } else if (m_mmb_down) {
                    m_pan_dx += (mx - m_last_mouse_x);
                    m_pan_dy += (my - m_last_mouse_y);
                    m_camera_dirty = true;
                }
                m_last_mouse_x = mx;
                m_last_mouse_y = my;
                return 0;
            }
            case WM_MOUSEWHEEL: {
                int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                m_scroll_delta += delta;
                m_camera_dirty = true;
                return 0;
            }
            break;
        }
        return StaticWindow::HandleMessage(msg, wParam, lParam);
    }

    void consume_input(int& out_dx, int& out_dy, int& out_scroll,
                       int& out_pan_dx, int& out_pan_dy) {
        out_dx = m_mouse_dx;
        out_dy = m_mouse_dy;
        out_scroll = m_scroll_delta;
        out_pan_dx = m_pan_dx;
        out_pan_dy = m_pan_dy;
        m_mouse_dx = m_mouse_dy = 0;
        m_scroll_delta = 0;
        m_pan_dx = m_pan_dy = 0;
    }

    bool is_camera_dirty() const {return m_camera_dirty;}
    void clear_dirty() {m_camera_dirty = false;}
    bool is_dragging() const { return m_lmb_down || m_mmb_down; }
};
#endif //RAYTRACER2_ORBIT_CAMERA_H