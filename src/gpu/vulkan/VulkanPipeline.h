#pragma once

#include "GpuResources.h"
#include "vulkan_helpers.h"

class VulkanDevice;


class VulkanPipelineLayout final : public IPipelineLayout {
public:
    VulkanPipelineLayout(VulkanDevice& device, const PipelineLayoutDesc& desc);
    ~VulkanPipelineLayout() override;

    void setDebugName(const std::string& name) override;
    [[nodiscard]] uint32_t bindGroupLayoutCount() const override { return m_bind_group_layout_count; }

    [[nodiscard]] vk::PipelineLayout native() const { return m_layout; }
private:
    VulkanDevice& m_device;
    vk::PipelineLayout m_layout;
    uint32_t m_bind_group_layout_count;
};


class VulkanGraphicsPipeline final : public IGraphicsPipeline {
public:
    VulkanGraphicsPipeline(VulkanDevice& device, const GraphicsPipelineDesc& desc);
    ~VulkanGraphicsPipeline() override;

    void setDebugName(const std::string& name) override;
    [[nodiscard]] IPipelineLayout& layout() override { return m_layout; }

    [[nodiscard]] VulkanPipelineLayout& vkLayout() { return m_layout; }
    [[nodiscard]] vk::Pipeline native() const { return m_pipeline; }
private:
    VulkanDevice& m_device;
    VulkanPipelineLayout& m_layout;
    vk::Pipeline m_pipeline;
};


class VulkanComputePipeline final : public IComputePipeline {
public:
    VulkanComputePipeline(VulkanDevice& device, const ComputePipelineDesc& desc);
    ~VulkanComputePipeline() override;

    void setDebugName(const std::string& name) override;
    [[nodiscard]] IPipelineLayout& layout() override { return m_layout; }

    [[nodiscard]] VulkanPipelineLayout& vkLayout() { return m_layout; }
    [[nodiscard]] vk::Pipeline native() const { return m_pipeline; }
private:
    VulkanDevice& m_device;
    VulkanPipelineLayout& m_layout;
    vk::Pipeline m_pipeline;
};
