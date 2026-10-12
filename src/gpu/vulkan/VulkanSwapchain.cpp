#include "VulkanSwapchain.h"

#include <algorithm>
#include <stdexcept>

#include "VulkanDevice.h"
#include "VulkanTexture.h"


namespace {

vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats, vk::Format desired) {
    for (const vk::SurfaceFormatKHR& format : formats) {
        if (format.format == desired && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return format;
    }
    for (const vk::SurfaceFormatKHR& format : formats) {
        if (vkutil::fromVk(format.format) != TextureFormat::Undefined)
            return format;
    }
    throw std::runtime_error("VulkanSwapchain: surface has no supported format");
}

vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& modes, bool vsync) {
    if (vsync)
        return vk::PresentModeKHR::eFifo;
    for (vk::PresentModeKHR preferred : {vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eImmediate}) {
        if (std::find(modes.begin(), modes.end(), preferred) != modes.end())
            return preferred;
    }
    return vk::PresentModeKHR::eFifo;
}

vk::CompositeAlphaFlagBitsKHR chooseCompositeAlpha(vk::CompositeAlphaFlagsKHR supported) {
    for (auto alpha : {vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::CompositeAlphaFlagBitsKHR::eInherit,
                       vk::CompositeAlphaFlagBitsKHR::ePreMultiplied, vk::CompositeAlphaFlagBitsKHR::ePostMultiplied}) {
        if (supported & alpha)
            return alpha;
    }
    return vk::CompositeAlphaFlagBitsKHR::eOpaque;
}

}


VulkanSwapchain::VulkanSwapchain(VulkanDevice& device, const SwapchainDesc& desc) : m_device(device), m_desc(desc) {
    m_surface = vkutil::createSurface(m_device.instance(), desc.surface);
    recreate();
}

VulkanSwapchain::~VulkanSwapchain() {
    m_device.waitIdle();
    destroyImages();
    m_device.waitIdle();

    vk::Device vk_device = m_device.native();
    for (vk::Semaphore semaphore : m_image_available)
        vk_device.destroySemaphore(semaphore);
    for (vk::Semaphore semaphore : m_render_finished)
        vk_device.destroySemaphore(semaphore);
    vk_device.destroySwapchainKHR(m_swapchain);
    m_device.instance().destroySurfaceKHR(m_surface);
}

ITexture* VulkanSwapchain::acquireNextImage() {
    // A second attempt covers the common case of the swapchain going out of date on acquire.
    for (int attempt = 0; attempt < 2; ++attempt) {
        if (m_needs_recreate || !m_swapchain)
            recreate();
        if (!m_swapchain || m_needs_recreate)
            return nullptr; // e.g. minimized: zero-sized surface

        const uint32_t semaphore = m_next_acquire_semaphore;
        vk::ResultValue<uint32_t> result(vk::Result::eErrorOutOfDateKHR, 0);
        try {
            result = m_device.native().acquireNextImageKHR(m_swapchain, UINT64_MAX, m_image_available[semaphore], nullptr);
        } catch (const vk::OutOfDateKHRError&) {
            m_needs_recreate = true;
            continue;
        }

        // A suboptimal image is still presentable; recreate on the next acquire.
        if (result.result == vk::Result::eSuboptimalKHR)
            m_needs_recreate = true;

        m_image_index = result.value;
        m_acquire_semaphore = semaphore;
        m_next_acquire_semaphore = (semaphore + 1) % static_cast<uint32_t>(m_image_available.size());
        m_acquire_pending = true;
        return m_images[m_image_index].get();
    }
    return nullptr;
}

ITexture& VulkanSwapchain::currentImage() {
    return *m_images.at(m_image_index);
}

void VulkanSwapchain::resize(uint32_t width, uint32_t height) {
    m_desc.extent = {width, height};
    m_needs_recreate = true;
    if (width != 0 && height != 0)
        recreate();
}

void VulkanSwapchain::setVSync(bool enabled) {
    if (m_desc.vsync == enabled)
        return;
    m_desc.vsync = enabled;
    m_needs_recreate = true;
}

bool VulkanSwapchain::consumeAcquire() {
    const bool pending = m_acquire_pending;
    m_acquire_pending = false;
    return pending;
}

void VulkanSwapchain::present(vk::Queue queue) {
    vk::Semaphore wait = signalSemaphore();
    vk::PresentInfoKHR info(wait, m_swapchain, m_image_index);
    try {
        if (queue.presentKHR(info) == vk::Result::eSuboptimalKHR)
            m_needs_recreate = true;
    } catch (const vk::OutOfDateKHRError&) {
        m_needs_recreate = true;
    }
}

void VulkanSwapchain::recreate() {
    vk::PhysicalDevice physical_device = m_device.physicalDevice();
    vk::Device vk_device = m_device.native();

    const vk::SurfaceCapabilitiesKHR caps = physical_device.getSurfaceCapabilitiesKHR(m_surface);

    vk::Extent2D extent = caps.currentExtent;
    if (extent.width == UINT32_MAX) {
        extent.width = std::clamp(m_desc.extent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
        extent.height = std::clamp(m_desc.extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);
    }
    if (extent.width == 0 || extent.height == 0) {
        m_needs_recreate = true;
        return;
    }

    // Nothing may still be using the old images or semaphores.
    m_device.waitIdle();

    const vk::SurfaceFormatKHR surface_format = chooseSurfaceFormat(
        physical_device.getSurfaceFormatsKHR(m_surface), vkutil::toVk(m_desc.format));
    const vk::PresentModeKHR present_mode = choosePresentMode(
        physical_device.getSurfacePresentModesKHR(m_surface), m_desc.vsync);

    uint32_t image_count = std::max(m_desc.image_count, caps.minImageCount);
    if (caps.maxImageCount > 0)
        image_count = std::min(image_count, caps.maxImageCount);

    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;
    if (caps.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst)
        usage |= vk::ImageUsageFlagBits::eTransferDst;

    vk::SwapchainKHR old_swapchain = m_swapchain;
    vk::SwapchainCreateInfoKHR info(
        {},
        m_surface,
        image_count,
        surface_format.format,
        surface_format.colorSpace,
        extent,
        1,
        usage,
        vk::SharingMode::eExclusive,
        {},
        caps.currentTransform,
        chooseCompositeAlpha(caps.supportedCompositeAlpha),
        present_mode,
        true,
        old_swapchain);
    m_swapchain = vk_device.createSwapchainKHR(info);

    destroyImages();
    m_device.waitIdle(); // flush the old image views before their swapchain goes away
    if (old_swapchain)
        vk_device.destroySwapchainKHR(old_swapchain);

    m_format = vkutil::fromVk(surface_format.format);
    m_extent = {extent.width, extent.height};

    TextureDesc texture_desc;
    texture_desc.type = TextureType::Texture2D;
    texture_desc.format = m_format;
    texture_desc.extent = {extent.width, extent.height, 1};
    texture_desc.usage = TextureUsage::ColorAttachment;
    if (usage & vk::ImageUsageFlagBits::eTransferDst)
        texture_desc.usage |= TextureUsage::CopyDst;

    for (vk::Image image : vk_device.getSwapchainImagesKHR(m_swapchain))
        m_images.push_back(std::make_unique<VulkanTexture>(m_device, texture_desc, image, *this));

    for (vk::Semaphore semaphore : m_image_available)
        vk_device.destroySemaphore(semaphore);
    for (vk::Semaphore semaphore : m_render_finished)
        vk_device.destroySemaphore(semaphore);
    m_image_available.clear();
    m_render_finished.clear();
    for (size_t i = 0; i < m_images.size() + 1; ++i)
        m_image_available.push_back(vk_device.createSemaphore({}));
    for (size_t i = 0; i < m_images.size(); ++i)
        m_render_finished.push_back(vk_device.createSemaphore({}));

    m_image_index = 0;
    m_acquire_semaphore = 0;
    m_next_acquire_semaphore = 0;
    m_acquire_pending = false;
    m_needs_recreate = false;
}

void VulkanSwapchain::destroyImages() {
    m_images.clear();
}
