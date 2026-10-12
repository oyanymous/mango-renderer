#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>

#include <utility>

#include "Window.h"
#include "win32_helpers.h"


namespace {

// Per-window state reachable from WindowProc through GWLP_USERDATA.
struct WindowCallbacks {
    Window::PaintCallback paint;
    Window::ResizeCallback resize;
};

}

struct Window::Impl {
    HMODULE m_hinstance;
    HWND m_hwnd;
    WNDCLASS m_wndclass;
    LPCWSTR m_classname;
    WindowCallbacks m_callbacks;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto* callbacks = reinterpret_cast<WindowCallbacks*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        if (callbacks && callbacks->resize) {
            callbacks->resize(LOWORD(lParam), HIWORD(lParam));
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;

    case WM_ERASEBKGND:
        // The paint callback covers the whole client area; skipping the GDI erase avoids flicker.
        if (callbacks && callbacks->paint)
            return 1;
        break;

    case WM_PAINT:
        if (callbacks && callbacks->paint) {
            callbacks->paint();
            ValidateRect(hwnd, nullptr);
            return 0;
        }
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            EndPaint(hwnd, &ps);
        }
    return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

Window::Window(const std::string& window_title) : m_impl(std::make_unique<Impl>()), m_window_title(window_title) {
    m_impl->m_classname = L"Mango Window";
    m_impl->m_hinstance = GetModuleHandle(nullptr);
    m_impl->m_wndclass = createWindowClass(m_impl->m_hinstance, WindowProc, m_impl->m_classname);
    m_impl->m_hwnd = createWindow(m_impl->m_hinstance, m_impl->m_classname, toWide(m_window_title).c_str());
    SetWindowLongPtrW(m_impl->m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&m_impl->m_callbacks));
}

Window::~Window() {
    SetWindowLongPtrW(m_impl->m_hwnd, GWLP_USERDATA, 0);
    UnregisterClassW(m_impl->m_classname, m_impl->m_hinstance);
    DestroyWindow(m_impl->m_hwnd);
}

void Window::display() {
    ShowWindow(m_impl->m_hwnd, SW_SHOWNORMAL);
}

void Window::setWindowTitle(const std::string& title) {
    SetWindowTextA(m_impl->m_hwnd, title.c_str());
}

NativeSurfaceHandle Window::nativeSurface() const {
    NativeSurfaceHandle handle;
    handle.window = m_impl->m_hwnd;
    handle.display = m_impl->m_hinstance;
    return handle;
}

FramebufferSize Window::framebufferSize() const {
    RECT rect = {};
    GetClientRect(m_impl->m_hwnd, &rect);
    return {static_cast<uint32_t>(rect.right - rect.left), static_cast<uint32_t>(rect.bottom - rect.top)};
}

void Window::setPaintCallback(PaintCallback callback) {
    m_impl->m_callbacks.paint = std::move(callback);
}

void Window::setResizeCallback(ResizeCallback callback) {
    m_impl->m_callbacks.resize = std::move(callback);
}
