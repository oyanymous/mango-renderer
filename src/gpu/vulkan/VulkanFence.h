#pragma once

#include "GpuResources.h"
#include "vulkan_helpers.h"

class VulkanDevice;


class VulkanFence final : public IFence {
public:
    VulkanFence(VulkanDevice& device, bool signaled);
    ~VulkanFence() override;

    void setDebugName(const std::string& name) override;

    bool wait(uint64_t timeout_ns) override;
    void reset() override;
    [[nodiscard]] bool isSignaled() const override;

    [[nodiscard]] vk::Fence native() const { return m_fence; }
private:
    VulkanDevice& m_device;
    vk::Fence m_fence;
};
