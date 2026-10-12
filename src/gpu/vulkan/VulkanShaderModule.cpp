#include "VulkanShaderModule.h"

#include <algorithm>
#include <stdexcept>

#include "VulkanDevice.h"


VulkanShaderModule::VulkanShaderModule(VulkanDevice& device, const ShaderModuleDesc& desc)
    : m_device(device), m_desc_stage(desc.stage), m_entry_point(desc.entry_point) {
    auto source = std::find_if(desc.sources.begin(), desc.sources.end(), [](const ShaderSource& s) {
        return s.language == ShaderLanguage::SPIRV;
    });
    if (source == desc.sources.end())
        throw std::runtime_error("VulkanShaderModule: no SPIR-V source provided");
    if (source->code.empty() || source->code.size() % sizeof(uint32_t) != 0)
        throw std::runtime_error("VulkanShaderModule: SPIR-V size must be a non-zero multiple of 4");

    if (desc.stage == ShaderStage::Vertex)
        m_stage = vk::ShaderStageFlagBits::eVertex;
    else if (desc.stage == ShaderStage::Fragment)
        m_stage = vk::ShaderStageFlagBits::eFragment;
    else if (desc.stage == ShaderStage::Compute)
        m_stage = vk::ShaderStageFlagBits::eCompute;
    else
        throw std::runtime_error("VulkanShaderModule: a shader module must have exactly one stage");

    vk::ShaderModuleCreateInfo info({}, source->code.size(), reinterpret_cast<const uint32_t*>(source->code.data()));
    m_module = m_device.native().createShaderModule(info);
}

VulkanShaderModule::~VulkanShaderModule() {
    // Shader modules are not referenced by command buffers, only by pipeline creation.
    m_device.native().destroyShaderModule(m_module);
}

void VulkanShaderModule::setDebugName(const std::string& name) {
    m_device.setObjectName(m_module, name);
}
