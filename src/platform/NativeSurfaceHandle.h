#pragma once

#include <cstdint>


// Opaque handles a graphics backend needs to present into a window.
//  - Windows: window = HWND, display = HINSTANCE
//  - macOS:   window = NSWindow*, view = NSView*, layer = CAMetalLayer*
struct NativeSurfaceHandle {
    void* window = nullptr;
    void* view = nullptr;
    void* display = nullptr;
    void* layer = nullptr;
};

struct FramebufferSize {
    uint32_t width = 0;
    uint32_t height = 0;
};
