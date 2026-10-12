#pragma once

#include "GpuResources.h"
#include "vulkan_helpers.h"

class VulkanDevice;


class VulkanBindGroupLayout final : public IBindGroupLayout {
public:
    VulkanBindGroupLayout(VulkanDevice& device, const BindGroupLayoutDesc& desc);
    ~VulkanBindGroupLayout() override;

    void setDebugName(const std::string& name) override;
    [[nodiscard]] const BindGroupLayoutDesc& desc() const override { return m_desc; }

    [[nodiscard]] vk::DescriptorSetLayout native() const { return m_layout; }
private:
    VulkanDevice& m_device;
    vk::DescriptorSetLayout m_layout;
    BindGroupLayoutDesc m_desc;
};


class VulkanBindGroup final : public IBindGroup {
public:
    VulkanBindGroup(VulkanDevice& device, const BindGroupDesc& desc);
    ~VulkanBindGroup() override;

    void setDebugName(const std::string& name) override;
    [[nodiscard]] IBindGroupLayout& layout() override { return m_layout; }

    [[nodiscard]] vk::DescriptorSet native() const { return m_set; }
private:
    VulkanDevice& m_device;
    VulkanBindGroupLayout& m_layout;
    vk::DescriptorSet m_set;
};
