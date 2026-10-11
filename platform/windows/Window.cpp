#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>

#include <utility>

#include "Window.h"
#include "win32_helpers.h"


struct Window::Impl {
    HMODULE m_hinstance;
    HWND m_hwnd;
    WNDCLASS m_wndclass;
    LPCWSTR m_classname;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
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

Window::Window(std::string window_title) : m_window_title(std::move(window_title)), m_impl(std::make_unique<Impl>()) {
    m_impl->m_classname = L"Mango Window";
    m_impl->m_hinstance = GetModuleHandle(nullptr);
    m_impl->m_wndclass = createWindowClass(m_impl->m_hinstance, WindowProc, m_impl->m_classname);
    m_impl->m_hwnd = createWindow(m_impl->m_hinstance, m_impl->m_classname, toWide(m_window_title).c_str());
}

Window::~Window() {
    UnregisterClassW(m_impl->m_classname, m_impl->m_hinstance);
    DestroyWindow(m_impl->m_hwnd);
}

void Window::display() {
    ShowWindow(m_impl->m_hwnd, SW_SHOWNORMAL);
}

void Window::setWindowTitle(const std::string& title) {
    SetWindowTextA(m_impl->m_hwnd, title.c_str());
}
