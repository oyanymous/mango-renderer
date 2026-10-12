#pragma once

#include <functional>
#include <memory>

#include "BaseWindow.h"
#include "NativeSurfaceHandle.h"

class Window: public BaseWindow {
public:
    using PaintCallback = std::function<void()>;
    using ResizeCallback = std::function<void(uint32_t width, uint32_t height)>;

    explicit Window(const std::string& window_title = "");
    ~Window() override;

    void display() override;
    void setWindowTitle(const std::string& title) override;

    [[nodiscard]] NativeSurfaceHandle nativeSurface() const;
    [[nodiscard]] FramebufferSize framebufferSize() const;

    // Called whenever the window contents need to be redrawn.
    void setPaintCallback(PaintCallback callback);
    // Called with the new framebuffer size (in pixels) when the window is resized.
    void setResizeCallback(ResizeCallback callback);
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    std::string m_window_title;
};
