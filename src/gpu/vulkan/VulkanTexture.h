#pragma once

#include "GpuResources.h"
#include "vulkan_helpers.h"

class VulkanDevice;
class VulkanSwapchain;


class VulkanTexture final : public ITexture {
public:
    // Creates and owns a new image.
    VulkanTexture(VulkanDevice& device, const TextureDesc& desc);
    // Wraps an image owned by a swapchain; only the view is owned.
    VulkanTexture(VulkanDevice& device, const TextureDesc& desc, vk::Image image, VulkanSwapchain& swapchain);
    ~VulkanTexture() override;

    void setDebugName(const std::string& name) override;

    [[nodiscard]] const TextureDesc& desc() const override { return m_desc; }
    [[nodiscard]] TextureFormat format() const override { return m_desc.format; }
    [[nodiscard]] Extent3D extent() const override { return m_desc.extent; }
    [[nodiscard]] uint32_t mipLevels() const override { return m_desc.mip_levels; }

    [[nodiscard]] vk::Image image() const { return m_image; }
    [[nodiscard]] vk::ImageView view() const { return m_view; }
    [[nodiscard]] vk::ImageSubresourceRange fullRange() const;

    // Layout the image will be in once all previously recorded commands have executed.
    [[nodiscard]] vk::ImageLayout layout() const { return m_layout; }
    void setLayout(vk::ImageLayout layout) { m_layout = layout; }

    // Non-null when this texture is a swapchain back buffer.
    [[nodiscard]] VulkanSwapchain* swapchain() const { return m_swapchain; }
private:
    void createView();

    VulkanDevice& m_device;
    vk::Image m_image;
    vk::ImageView m_view;
    VmaAllocation m_allocation = nullptr;
    vk::ImageLayout m_layout = vk::ImageLayout::eUndefined;
    TextureDesc m_desc;
    bool m_owns_image;
    VulkanSwapchain* m_swapchain = nullptr;
};


class VulkanSampler final : public ISampler {
public:
    VulkanSampler(VulkanDevice& device, const SamplerDesc& desc);
    ~VulkanSampler() override;

    void setDebugName(const std::string& name) override;
    [[nodiscard]] const SamplerDesc& desc() const override { return m_desc; }

    [[nodiscard]] vk::Sampler native() const { return m_sampler; }
private:
    VulkanDevice& m_device;
    vk::Sampler m_sampler;
    SamplerDesc m_desc;
};
