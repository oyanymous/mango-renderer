#include "VulkanFence.h"

#include "VulkanDevice.h"


VulkanFence::VulkanFence(VulkanDevice& device, bool signaled) : m_device(device) {
    vk::FenceCreateFlags flags = signaled ? vk::FenceCreateFlagBits::eSignaled : vk::FenceCreateFlags();
    m_fence = m_device.native().createFence(vk::FenceCreateInfo(flags));
}

VulkanFence::~VulkanFence() {
    m_device.deferDestroy([device = m_device.native(), fence = m_fence] { device.destroyFence(fence); });
}

void VulkanFence::setDebugName(const std::string& name) {
    m_device.setObjectName(m_fence, name);
}

bool VulkanFence::wait(uint64_t timeout_ns) {
    return m_device.native().waitForFences(m_fence, true, timeout_ns) == vk::Result::eSuccess;
}

void VulkanFence::reset() {
    m_device.native().resetFences(m_fence);
}

bool VulkanFence::isSignaled() const {
    return m_device.native().getFenceStatus(m_fence) == vk::Result::eSuccess;
}
