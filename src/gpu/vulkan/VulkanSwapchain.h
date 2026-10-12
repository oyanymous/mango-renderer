#pragma once

#include <memory>
#include <vector>

#include "ISwapchain.h"
#include "vulkan_helpers.h"

class VulkanDevice;
class VulkanTexture;


class VulkanSwapchain final : public ISwapchain {
public:
    VulkanSwapchain(VulkanDevice& device, const SwapchainDesc& desc);
    ~VulkanSwapchain() override;

    ITexture* acquireNextImage() override;
    ITexture& currentImage() override;
    void resize(uint32_t width, uint32_t height) override;
    [[nodiscard]] TextureFormat format() const override { return m_format; }
    [[nodiscard]] Extent2D extent() const override { return m_extent; }
    void setVSync(bool enabled) override;

    [[nodiscard]] vk::SwapchainKHR native() const { return m_swapchain; }

    // Semaphore signaled by the last acquire; the submit that renders to the image waits on it.
    [[nodiscard]] vk::Semaphore waitSemaphore() const { return m_image_available[m_acquire_semaphore]; }
    // Semaphore the rendering submit signals and present() waits on.
    [[nodiscard]] vk::Semaphore signalSemaphore() const { return m_render_finished[m_image_index]; }
    // True exactly once after each successful acquire, so only one submit waits on it.
    bool consumeAcquire();

    void present(vk::Queue queue);
private:
    void recreate();
    void destroyImages();

    VulkanDevice& m_device;
    SwapchainDesc m_desc;
    vk::SurfaceKHR m_surface;
    vk::SwapchainKHR m_swapchain;
    std::vector<std::unique_ptr<VulkanTexture>> m_images;
    // Indexed by acquire count (ring); one per image plus one spare.
    std::vector<vk::Semaphore> m_image_available;
    // Indexed by image, so a semaphore is only reused once its image is presented again.
    std::vector<vk::Semaphore> m_render_finished;
    uint32_t m_image_index = 0;
    uint32_t m_acquire_semaphore = 0;
    uint32_t m_next_acquire_semaphore = 0;
    bool m_acquire_pending = false;
    bool m_needs_recreate = false;
    TextureFormat m_format = TextureFormat::Undefined;
    Extent2D m_extent;
};
