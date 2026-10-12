#pragma once

#include "ICommandQueue.h"
#include "vulkan_helpers.h"

class VulkanDevice;


class VulkanCommandQueue final : public ICommandQueue {
public:
    VulkanCommandQueue(VulkanDevice& device, uint32_t family_index);
    ~VulkanCommandQueue() override;

    std::unique_ptr<ICommandBuffer> createCommandBuffer() override;
    void submit(std::span<ICommandBuffer* const> buffers, IFence* signal_fence) override;
    void present(ISwapchain& swapchain) override;
    void waitIdle() override;

    [[nodiscard]] vk::Queue native() const { return m_queue; }
    [[nodiscard]] uint32_t familyIndex() const { return m_family_index; }
    [[nodiscard]] vk::CommandPool commandPool() const { return m_command_pool; }

    // Every submit signals a timeline semaphore with an increasing value; the deletion
    // queue uses it to know when resources are no longer referenced by the GPU.
    [[nodiscard]] uint64_t lastSubmittedValue() const { return m_timeline_value; }
    [[nodiscard]] uint64_t completedValue() const;

    // Submits a single raw command buffer and blocks until it has executed.
    void submitAndWait(vk::CommandBuffer command_buffer);
private:
    VulkanDevice& m_device;
    vk::Queue m_queue;
    uint32_t m_family_index;
    vk::CommandPool m_command_pool;
    vk::Semaphore m_timeline;
    uint64_t m_timeline_value = 0;
};
