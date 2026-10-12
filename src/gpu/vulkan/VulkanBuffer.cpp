#include "VulkanBuffer.h"

#include <cstring>
#include <stdexcept>

#include "VulkanDevice.h"


VulkanBuffer::VulkanBuffer(VulkanDevice& device, const BufferDesc& desc, const void* initial_data)
    : m_device(device), m_desc(desc) {
    // GPU-only buffers are filled through a staging copy, so they must be copy destinations.
    BufferUsage usage = m_desc.usage;
    if (m_desc.memory == MemoryType::GpuOnly)
        usage |= BufferUsage::CopyDst;

    VkBufferCreateInfo buffer_info = vk::BufferCreateInfo({}, m_desc.size, vkutil::toVk(usage));

    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    switch (m_desc.memory) {
    case MemoryType::GpuOnly:
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        break;
    case MemoryType::CpuToGpu:
        alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        break;
    case MemoryType::GpuToCpu:
        alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        break;
    }

    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocationInfo allocation_info = {};
    if (vmaCreateBuffer(m_device.allocator(), &buffer_info, &alloc_info, &buffer, &m_allocation, &allocation_info) != VK_SUCCESS)
        throw std::runtime_error("VulkanBuffer: vmaCreateBuffer failed");
    m_buffer = buffer;
    m_mapped = allocation_info.pMappedData;

    if (initial_data)
        update(initial_data, m_desc.size, 0);
}

VulkanBuffer::~VulkanBuffer() {
    m_device.deferDestroy([allocator = m_device.allocator(), buffer = m_buffer, allocation = m_allocation] {
        vmaDestroyBuffer(allocator, buffer, allocation);
    });
}

void VulkanBuffer::setDebugName(const std::string& name) {
    m_device.setObjectName(m_buffer, name);
}

void VulkanBuffer::unmap() {
    // Host-visible buffers stay persistently mapped; just make writes visible on non-coherent memory.
    if (m_mapped)
        vmaFlushAllocation(m_device.allocator(), m_allocation, 0, VK_WHOLE_SIZE);
}

void VulkanBuffer::update(const void* data, uint64_t size, uint64_t offset) {
    if (m_mapped) {
        std::memcpy(static_cast<uint8_t*>(m_mapped) + offset, data, size);
        vmaFlushAllocation(m_device.allocator(), m_allocation, offset, size);
        return;
    }

    // Device-local memory: go through a temporary staging buffer.
    VulkanBuffer staging(m_device, BufferDesc{size, BufferUsage::CopySrc, MemoryType::CpuToGpu}, data);
    m_device.immediateSubmit([&](vk::CommandBuffer command_buffer) {
        command_buffer.copyBuffer(staging.native(), m_buffer, vk::BufferCopy(0, offset, size));
    });
}
