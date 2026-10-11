#include "win32_helpers.h"


WNDCLASSW createWindowClass(const HINSTANCE& hInstance, const WNDPROC& wndProc, const LPCWSTR& className) {
    WNDCLASSW wc = {};

    wc.lpfnWndProc   = wndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = className;

    RegisterClassW(&wc);
    return wc;
}

HWND createWindow(const HINSTANCE& hInstance, const LPCWSTR& className, const LPCWSTR& windowTitle) {
    HWND hwnd = CreateWindowExW(
        0,                              // Optional window styles.
        className,                     // Window class
        windowTitle,    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );
    return hwnd;
}

std::wstring toWide(const std::string& str) {
    if (str.empty())
        return {};

    // First call: ask how many wchar_t's are needed
    const int size = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);

    // Second call: do the actual conversion into a buffer of that size
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), size);
    return result;
}
