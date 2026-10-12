#pragma once

#include <cstdint>

#include "GpuResources.h"


class ISwapchain {
public:
    virtual ~ISwapchain() = default;

    // Returns nullptr when no image can be acquired this frame (e.g. the window is
    // minimized or the swapchain had to be recreated); skip the frame in that case.
    virtual ITexture* acquireNextImage() = 0;
    virtual ITexture& currentImage() = 0;
    virtual void resize(uint32_t width, uint32_t height) = 0;
    [[nodiscard]] virtual TextureFormat format() const = 0;
    [[nodiscard]] virtual Extent2D extent() const = 0;
    virtual void setVSync(bool enabled) = 0;
};
