#pragma once

#include "GpuResources.h"
#include "vulkan_helpers.h"

class VulkanDevice;


class VulkanShaderModule final : public IShaderModule {
public:
    // Picks the SPIR-V entry of `desc.sources`; throws if there is none.
    VulkanShaderModule(VulkanDevice& device, const ShaderModuleDesc& desc);
    ~VulkanShaderModule() override;

    void setDebugName(const std::string& name) override;

    [[nodiscard]] ShaderStage stage() const override { return m_desc_stage; }
    [[nodiscard]] const std::string& entryPoint() const override { return m_entry_point; }

    [[nodiscard]] vk::ShaderModule native() const { return m_module; }
    [[nodiscard]] vk::ShaderStageFlagBits vkStage() const { return m_stage; }
private:
    VulkanDevice& m_device;
    vk::ShaderModule m_module;
    vk::ShaderStageFlagBits m_stage;
    ShaderStage m_desc_stage;
    std::string m_entry_point;
};
