//
// Created by vivek on 10/19/2025.
//

#ifndef WIN32DISPLAY_H
#define WIN32DISPLAY_H
#ifndef UNICODE
#define UNICODE
#include <string>
#endif
#ifndef _UNICODE
#define _UNICODE
#endif


#include <windows.h>

#include <cstdint>
#include <fstream>
#include <memory>
#include "vmath.h"

class Win32Display {
public:
    Win32Display(int width, int height, const wchar_t* title) : m_width(width), m_height(height),
                                                                m_window_title(title) {}

    ~Win32Display() {
        if (m_hdc_offscreen) DeleteDC(m_hdc_offscreen);
        if (m_h_bitmap) DeleteObject(m_h_bitmap);
    }

    bool create() {
        HINSTANCE hInstance = GetModuleHandle(NULL);

        WNDCLASSEX wc = {0};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = Win32Display::WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"GenericWindowClass";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

        if (!RegisterClassEx(&wc)) return false;

        m_hwnd = CreateWindowEx(
            WS_EX_CLIENTEDGE, wc.lpszClassName, m_window_title,
            WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, m_width, m_height,
            nullptr, nullptr, hInstance, this);

        if (!m_hwnd) return false;

        initialize_buffer();

        ShowWindow(m_hwnd, SW_SHOWDEFAULT);
        UpdateWindow(m_hwnd);

        return true;
    }

    bool process_messages() {
        MSG msg = {};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return true;
    }

    void update_window() {
        // copy software buffer to DIB buffer
        std::memcpy(m_p_bits, m_pixel_buffer.get(), m_width*m_height*3);

        // invalidate the window to trigger WM_PAINT message
        InvalidateRect(m_hwnd, NULL, FALSE);
        UpdateWindow(m_hwnd);
    }

    uchar* get_pixel_buffer() {
        return m_pixel_buffer.get();
    }

    // Helper to set a single pixel's color. Note: This is slow if called per-pixel.
    // It's often faster to get the buffer pointer and write to it directly in a loop.
    void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;
        int index = (y * m_width + x) * 3;
        m_pixel_buffer[index + 0] = b; // BGR order for Windows
        m_pixel_buffer[index + 1] = g;
        m_pixel_buffer[index + 2] = r;
    }

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        Win32Display* pThis = nullptr;

        if (msg == WM_NCCREATE) {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (Win32Display*)pCreate->lpCreateParams;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
            pThis->m_hwnd = hWnd;
        } else {
            pThis = (Win32Display*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        }

        if (pThis) {
            return pThis->handle_message(msg, wParam, lParam);
        }

        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    void SaveAsPPM(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::clog << "Failed to open file for writing: " << filename << "\n";
            return;
        }

        file << "P3\n" << m_width << " " << m_height << "\n255\n";
        uchar* pb = get_pixel_buffer();

        for (int j = 0; j < m_height; j++) {
            for (int i = 0; i < m_width; i++) {
                int index = (j * m_width + i) * 3;
                uint8_t b = pb[index + 0]; // BGR order for Windows
                uint8_t g = pb[index + 1];
                uint8_t r = pb[index + 2];
                // if (index%1000 == 0)
                //     std::clog << (int)r << " " << (int)g << " " << (int)b << "\n";
                file << (int)r << " " << (int)g << " " << (int)b << "\n";
            }
        }

        file.close();
        std::clog << "Saved render as: " << filename << "\n";
    }

    LRESULT handle_message(UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hwnd, &ps);
            BitBlt(hdc, 0, 0, m_width, m_height, m_hdc_offscreen, 0, 0, SRCCOPY);
            EndPaint(m_hwnd, &ps);
            return 0;
        }
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE || wParam == 'Q' ) {
                std::clog << "Quitting" << "\n";
                DestroyWindow(m_hwnd);
                return 0;
            } else if (wParam == 'S') {
                std::string fname = "sh_render" + std::to_string(m_width) + "x" + std::to_string(m_height) + ".ppm";
                SaveAsPPM(fname);
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(m_hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_ERASEBKGND:
            return 1;

        }

        return DefWindowProc(m_hwnd, msg, wParam, lParam);

    }

    void initialize_buffer() {
        m_pixel_buffer = std::make_unique<uchar[]>(m_width * m_height * 3);

        BITMAPINFO bmi = {0};
        bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
        bmi.bmiHeader.biWidth = m_width;
        bmi.bmiHeader.biHeight = -m_height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;

        HDC hdc = GetDC(m_hwnd);
        m_h_bitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &m_p_bits, nullptr, 0);

        m_hdc_offscreen = CreateCompatibleDC(hdc);
        SelectObject(m_hdc_offscreen, m_h_bitmap);
        ReleaseDC(m_hwnd, hdc);

    }

    HWND m_hwnd = nullptr;
    HBITMAP m_h_bitmap = nullptr;
    HDC m_hdc_offscreen = nullptr;
    void* m_p_bits = nullptr;
    std::unique_ptr<uchar[]> m_pixel_buffer;

    int m_width;
    int m_height;
    const wchar_t* m_window_title;
};


#endif //HTBBVH_WIN32DISPLAY_H
