//
// Created by vivek on 1/31/2026.
//
#ifndef RAYTRACER2_BASE_WIN32_H
#define RAYTRACER2_BASE_WIN32_H
#include <iostream>
#include <memory>
#include <windows.h>

template <class DERIVED_TYPE>
class BaseWindow {
public:
    BaseWindow() : m_hwnd(NULL) {}

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        DERIVED_TYPE *pThis = NULL;

        if (uMsg == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (DERIVED_TYPE*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

            pThis->m_hwnd = hwnd;
        }
        else
        {
            pThis = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        if (pThis)
        {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    bool create(
        PCWSTR lpWindowName,
        DWORD dwStyle,
        DWORD dwExStyle = 0,
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int nWidth = CW_USEDEFAULT,
        int nHeight= CW_USEDEFAULT,
        HWND hWndParent = 0,
        HMENU hMenu = 0
        )
    {
        WNDCLASSEX wc = {0};

        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = DERIVED_TYPE::WindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = ClassName();

        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

        if (!RegisterClassEx(&wc)) return false;

        m_hwnd = CreateWindowEx(dwExStyle, wc.lpszClassName, lpWindowName, dwStyle, x, y,
                                nWidth, nHeight, hWndParent, hMenu, wc.hInstance, this);

        return (m_hwnd ? TRUE : FALSE);
    }

    HWND Window() const {return m_hwnd;}

    bool show() {
        MSG msg = {};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return false;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return true;
    }

protected:
    virtual PCWSTR ClassName() const = 0;
    virtual LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) = 0;

    HWND m_hwnd;
};

class StaticWindow : public BaseWindow<StaticWindow> {
public:
    StaticWindow(const int width, const int height, const wchar_t* title) : m_width(width), m_height(height), m_window_title(title) {}
    ~StaticWindow() {
        if (m_hdc_offscreen) DeleteDC(m_hdc_offscreen);
        if (m_h_bitmap) DeleteObject(m_h_bitmap);
    }

    PCWSTR ClassName() const override {return m_window_title;};

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) override {
        switch (msg) {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(m_hwnd, &ps);
                BitBlt(hdc, 0, 0, m_width, m_height, m_hdc_offscreen, 0, 0, SRCCOPY);
                EndPaint(m_hwnd, &ps);
                return 0;
            }

            case WM_KEYDOWN:
                if (wParam == VK_ESCAPE || wParam == 'Q') {
                    std::clog << "Quitting" << "\n";
                    DestroyWindow(m_hwnd);
                    return 0;
                }
                break;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            case WM_ERASEBKGND:
                return 1;
        }
        return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }

    void init_pixel_buffer() {
        m_pixel_buffer = std::make_unique<unsigned char[]>(m_width * m_height * 3);
        std::memset(m_pixel_buffer.get(), 0, m_width * m_height * 3);
    }

    void init_dib() {
        BITMAPINFO bmi = config_bmi();
        HDC hdc = GetDC(m_hwnd);
        m_h_bitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &m_p_bits, nullptr, 0);
        m_hdc_offscreen = CreateCompatibleDC(hdc);
        SelectObject(m_hdc_offscreen, m_h_bitmap);
        ReleaseDC(m_hwnd, hdc);
    }

    unsigned char* get_pixel_buffer() {
        return m_pixel_buffer.get();
    }

    void update_window() {
        // copy software buffer to DIB buffer
        std::memcpy(m_p_bits, m_pixel_buffer.get(), m_width*m_height*3);

        // invalidate the window to trigger WM_PAINT message
        InvalidateRect(m_hwnd, NULL, FALSE);
        UpdateWindow(m_hwnd);
    }

    int m_width;
    int m_height;
    PCWSTR m_window_title;
    std::unique_ptr<unsigned char[]> m_pixel_buffer;
private:
    BITMAPINFO config_bmi() const {
        BITMAPINFO bmi = {0};
        bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
        bmi.bmiHeader.biWidth = m_width;
        bmi.bmiHeader.biHeight = -m_height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;
        return bmi;
    }

    HBITMAP m_h_bitmap = nullptr;
    HDC m_hdc_offscreen = nullptr;
    void* m_p_bits = nullptr;

};

#endif //RAYTRACER2_BASE_WIN32_H