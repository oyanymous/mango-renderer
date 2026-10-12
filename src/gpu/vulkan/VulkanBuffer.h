#pragma once

#include "GpuResources.h"
#include "vulkan_helpers.h"

class VulkanDevice;


class VulkanBuffer final : public IBuffer {
public:
    VulkanBuffer(VulkanDevice& device, const BufferDesc& desc, const void* initial_data);
    ~VulkanBuffer() override;

    void setDebugName(const std::string& name) override;

    [[nodiscard]] uint64_t size() const override { return m_desc.size; }
    [[nodiscard]] BufferUsage usage() const override { return m_desc.usage; }
    void* map() override { return m_mapped; }
    void unmap() override;
    void update(const void* data, uint64_t size, uint64_t offset) override;

    [[nodiscard]] vk::Buffer native() const { return m_buffer; }
private:
    VulkanDevice& m_device;
    BufferDesc m_desc;
    vk::Buffer m_buffer;
    VmaAllocation m_allocation = nullptr;
    void* m_mapped = nullptr;
};
