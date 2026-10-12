#include "VulkanCommandQueue.h"

#include <vector>

#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanFence.h"
#include "VulkanSwapchain.h"


VulkanCommandQueue::VulkanCommandQueue(VulkanDevice& device, uint32_t family_index)
    : m_device(device), m_family_index(family_index) {
    vk::Device vk_device = m_device.native();
    m_queue = vk_device.getQueue(family_index, 0);

    m_command_pool = vk_device.createCommandPool(
        vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, family_index));

    vk::SemaphoreTypeCreateInfo timeline_info(vk::SemaphoreType::eTimeline, 0);
    m_timeline = vk_device.createSemaphore(vk::SemaphoreCreateInfo({}, &timeline_info));
}

VulkanCommandQueue::~VulkanCommandQueue() {
    vk::Device vk_device = m_device.native();
    vk_device.destroySemaphore(m_timeline);
    vk_device.destroyCommandPool(m_command_pool);
}

std::unique_ptr<ICommandBuffer> VulkanCommandQueue::createCommandBuffer() {
    vk::CommandBufferAllocateInfo info(m_command_pool, vk::CommandBufferLevel::ePrimary, 1);
    vk::CommandBuffer command_buffer = m_device.native().allocateCommandBuffers(info).front();
    return std::make_unique<VulkanCommandBuffer>(m_device, command_buffer);
}

void VulkanCommandQueue::submit(std::span<ICommandBuffer* const> buffers, IFence* signal_fence) {
    std::vector<vk::CommandBufferSubmitInfo> command_buffers;
    std::vector<vk::SemaphoreSubmitInfo> waits;
    std::vector<vk::SemaphoreSubmitInfo> signals;

    for (ICommandBuffer* buffer : buffers) {
        auto* vk_buffer = static_cast<VulkanCommandBuffer*>(buffer);
        command_buffers.emplace_back(vk_buffer->native());

        // Rendering into a swapchain image must wait for the acquire and signal the
        // semaphore that present() waits on.
        for (VulkanSwapchain* swapchain : vk_buffer->swapchains()) {
            if (!swapchain->consumeAcquire())
                continue;
            waits.emplace_back(swapchain->waitSemaphore(), 0, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
            signals.emplace_back(swapchain->signalSemaphore(), 0, vk::PipelineStageFlagBits2::eAllCommands);
        }
    }

    signals.emplace_back(m_timeline, ++m_timeline_value, vk::PipelineStageFlagBits2::eAllCommands);

    vk::SubmitInfo2 submit_info({}, waits, command_buffers, signals);
    vk::Fence fence = signal_fence ? static_cast<VulkanFence*>(signal_fence)->native() : vk::Fence();
    m_queue.submit2(submit_info, fence);

    m_device.collectGarbage();
}

void VulkanCommandQueue::present(ISwapchain& swapchain) {
    static_cast<VulkanSwapchain&>(swapchain).present(m_queue);
}

void VulkanCommandQueue::waitIdle() {
    m_queue.waitIdle();
}

uint64_t VulkanCommandQueue::completedValue() const {
    return m_device.native().getSemaphoreCounterValue(m_timeline);
}

void VulkanCommandQueue::submitAndWait(vk::CommandBuffer command_buffer) {
    vk::CommandBufferSubmitInfo command_info(command_buffer);
    vk::SemaphoreSubmitInfo signal(m_timeline, ++m_timeline_value, vk::PipelineStageFlagBits2::eAllCommands);
    m_queue.submit2(vk::SubmitInfo2({}, {}, command_info, signal));

    vk::SemaphoreWaitInfo wait_info({}, m_timeline, m_timeline_value);
    (void) m_device.native().waitSemaphores(wait_info, UINT64_MAX);
}
