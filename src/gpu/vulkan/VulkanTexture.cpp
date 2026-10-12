#include "VulkanTexture.h"

#include <stdexcept>

#include "VulkanDevice.h"


namespace {

vk::ImageType imageType(TextureType type) {
    switch (type) {
    case TextureType::Texture1D: return vk::ImageType::e1D;
    case TextureType::Texture3D: return vk::ImageType::e3D;
    default: return vk::ImageType::e2D;
    }
}

vk::ImageViewType viewType(TextureType type) {
    switch (type) {
    case TextureType::Texture1D: return vk::ImageViewType::e1D;
    case TextureType::Texture2D: return vk::ImageViewType::e2D;
    case TextureType::Texture3D: return vk::ImageViewType::e3D;
    case TextureType::TextureCube: return vk::ImageViewType::eCube;
    case TextureType::Texture2DArray: return vk::ImageViewType::e2DArray;
    }
    return vk::ImageViewType::e2D;
}

}


VulkanTexture::VulkanTexture(VulkanDevice& device, const TextureDesc& desc)
    : m_device(device), m_desc(desc), m_owns_image(true) {
    vk::ImageCreateFlags flags;
    if (m_desc.type == TextureType::TextureCube)
        flags |= vk::ImageCreateFlagBits::eCubeCompatible;

    VkImageCreateInfo image_info = vk::ImageCreateInfo(
        flags,
        imageType(m_desc.type),
        vkutil::toVk(m_desc.format),
        vk::Extent3D(m_desc.extent.width, m_desc.extent.height, m_desc.extent.depth),
        m_desc.mip_levels,
        m_desc.array_layers,
        vkutil::toVkSampleCount(m_desc.sample_count),
        vk::ImageTiling::eOptimal,
        vkutil::toVk(m_desc.usage),
        vk::SharingMode::eExclusive,
        {},
        vk::ImageLayout::eUndefined);

    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    VkImage image = VK_NULL_HANDLE;
    if (vmaCreateImage(m_device.allocator(), &image_info, &alloc_info, &image, &m_allocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("VulkanTexture: vmaCreateImage failed");
    m_image = image;

    createView();
}

VulkanTexture::VulkanTexture(VulkanDevice& device, const TextureDesc& desc, vk::Image image, VulkanSwapchain& swapchain)
    : m_device(device), m_image(image), m_desc(desc), m_owns_image(false), m_swapchain(&swapchain) {
    createView();
}

VulkanTexture::~VulkanTexture() {
    m_device.deferDestroy([device = m_device.native(), allocator = m_device.allocator(), view = m_view,
                           image = m_image, allocation = m_allocation, owns_image = m_owns_image] {
        device.destroyImageView(view);
        if (owns_image)
            vmaDestroyImage(allocator, image, allocation);
    });
}

void VulkanTexture::setDebugName(const std::string& name) {
    m_device.setObjectName(m_image, name);
    m_device.setObjectName(m_view, name + " (view)");
}

vk::ImageSubresourceRange VulkanTexture::fullRange() const {
    return {vkutil::aspectMask(m_desc.format), 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};
}

void VulkanTexture::createView() {
    vk::ImageViewCreateInfo view_info({}, m_image, viewType(m_desc.type), vkutil::toVk(m_desc.format), {}, fullRange());
    m_view = m_device.native().createImageView(view_info);
}


VulkanSampler::VulkanSampler(VulkanDevice& device, const SamplerDesc& desc) : m_device(device), m_desc(desc) {
    vk::SamplerCreateInfo info(
        {},
        vkutil::toVk(desc.mag_filter),
        vkutil::toVk(desc.min_filter),
        vkutil::toVkMipmapMode(desc.mip_filter),
        vkutil::toVk(desc.address_u),
        vkutil::toVk(desc.address_v),
        vkutil::toVk(desc.address_w),
        0.0f,
        desc.max_anisotropy > 1.0f,
        desc.max_anisotropy,
        desc.compare.has_value(),
        desc.compare ? vkutil::toVk(*desc.compare) : vk::CompareOp::eAlways,
        0.0f,
        VK_LOD_CLAMP_NONE);
    m_sampler = m_device.native().createSampler(info);
}

VulkanSampler::~VulkanSampler() {
    m_device.deferDestroy([device = m_device.native(), sampler = m_sampler] { device.destroySampler(sampler); });
}

void VulkanSampler::setDebugName(const std::string& name) {
    m_device.setObjectName(m_sampler, name);
}
