#include "VulkanPipeline.h"

#include <array>
#include <stdexcept>
#include <vector>

#include "VulkanBindGroup.h"
#include "VulkanDevice.h"
#include "VulkanShaderModule.h"


namespace {

VulkanPipelineLayout& requireLayout(IPipelineLayout* layout) {
    if (layout == nullptr)
        throw std::runtime_error("Vulkan pipeline: a pipeline layout is required");
    return *static_cast<VulkanPipelineLayout*>(layout);
}

vk::PipelineShaderStageCreateInfo stageInfo(IShaderModule& module) {
    auto& vk_module = static_cast<VulkanShaderModule&>(module);
    return {{}, vk_module.vkStage(), vk_module.native(), vk_module.entryPoint().c_str()};
}

}


VulkanPipelineLayout::VulkanPipelineLayout(VulkanDevice& device, const PipelineLayoutDesc& desc)
    : m_device(device), m_bind_group_layout_count(static_cast<uint32_t>(desc.bind_group_layouts.size())) {
    std::vector<vk::DescriptorSetLayout> set_layouts;
    for (IBindGroupLayout* layout : desc.bind_group_layouts)
        set_layouts.push_back(static_cast<VulkanBindGroupLayout*>(layout)->native());

    std::vector<vk::PushConstantRange> push_constants;
    for (const PushConstantRange& range : desc.push_constants)
        push_constants.emplace_back(vkutil::toVk(range.stages), range.offset, range.size);

    m_layout = m_device.native().createPipelineLayout(vk::PipelineLayoutCreateInfo({}, set_layouts, push_constants));
}

VulkanPipelineLayout::~VulkanPipelineLayout() {
    m_device.deferDestroy([device = m_device.native(), layout = m_layout] { device.destroyPipelineLayout(layout); });
}

void VulkanPipelineLayout::setDebugName(const std::string& name) {
    m_device.setObjectName(m_layout, name);
}


VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice& device, const GraphicsPipelineDesc& desc)
    : m_device(device), m_layout(requireLayout(desc.layout)) {
    if (desc.vertex_shader == nullptr)
        throw std::runtime_error("VulkanGraphicsPipeline: a vertex shader is required");

    std::vector<vk::PipelineShaderStageCreateInfo> stages;
    stages.push_back(stageInfo(*desc.vertex_shader));
    if (desc.fragment_shader)
        stages.push_back(stageInfo(*desc.fragment_shader));

    std::vector<vk::VertexInputBindingDescription> bindings;
    for (const VertexBufferLayout& buffer : desc.vertex_layout.buffers)
        bindings.emplace_back(buffer.binding, buffer.stride, vkutil::toVk(buffer.step_mode));

    std::vector<vk::VertexInputAttributeDescription> attributes;
    for (const VertexAttribute& attribute : desc.vertex_layout.attributes)
        attributes.emplace_back(attribute.location, attribute.binding, vkutil::toVk(attribute.format), attribute.offset);

    vk::PipelineVertexInputStateCreateInfo vertex_input({}, bindings, attributes);
    vk::PipelineInputAssemblyStateCreateInfo input_assembly({}, vkutil::toVk(desc.topology));

    // Viewport and scissor are set from the command buffer.
    vk::PipelineViewportStateCreateInfo viewport_state({}, 1, nullptr, 1, nullptr);

    vk::PipelineRasterizationStateCreateInfo raster(
        {},
        desc.raster.depth_clamp,
        false,
        vkutil::toVk(desc.raster.polygon_mode),
        vkutil::toVk(desc.raster.cull_mode),
        vkutil::toVk(desc.raster.front_face),
        false, 0.0f, 0.0f, 0.0f,
        1.0f);

    vk::PipelineMultisampleStateCreateInfo multisample({}, vkutil::toVkSampleCount(desc.sample_count));

    vk::PipelineDepthStencilStateCreateInfo depth_stencil(
        {},
        desc.depth_stencil.depth_test,
        desc.depth_stencil.depth_write,
        vkutil::toVk(desc.depth_stencil.depth_compare));

    std::vector<vk::PipelineColorBlendAttachmentState> blend_attachments;
    for (size_t i = 0; i < desc.color_formats.size(); ++i) {
        const BlendState blend = i < desc.blend.size() ? desc.blend[i] : BlendState{};
        blend_attachments.emplace_back(
            blend.enabled,
            vkutil::toVk(blend.src_color), vkutil::toVk(blend.dst_color), vkutil::toVk(blend.color_op),
            vkutil::toVk(blend.src_alpha), vkutil::toVk(blend.dst_alpha), vkutil::toVk(blend.alpha_op),
            vkutil::toVk(blend.write_mask));
    }
    vk::PipelineColorBlendStateCreateInfo color_blend({}, false, vk::LogicOp::eCopy, blend_attachments);

    std::array dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamic_state({}, dynamic_states);

    // Dynamic rendering: attachment formats replace the VkRenderPass.
    std::vector<vk::Format> color_formats;
    for (TextureFormat format : desc.color_formats)
        color_formats.push_back(vkutil::toVk(format));
    vk::Format depth_format = isDepthFormat(desc.depth_format) ? vkutil::toVk(desc.depth_format) : vk::Format::eUndefined;
    vk::Format stencil_format = hasStencil(desc.depth_format) ? depth_format : vk::Format::eUndefined;
    vk::PipelineRenderingCreateInfo rendering_info(0, color_formats, depth_format, stencil_format);

    vk::GraphicsPipelineCreateInfo info(
        {},
        stages,
        &vertex_input,
        &input_assembly,
        nullptr,
        &viewport_state,
        &raster,
        &multisample,
        &depth_stencil,
        &color_blend,
        &dynamic_state,
        m_layout.native());
    info.setPNext(&rendering_info);

    auto [result, pipeline] = m_device.native().createGraphicsPipeline(nullptr, info);
    if (result != vk::Result::eSuccess)
        throw std::runtime_error("VulkanGraphicsPipeline: pipeline creation failed");
    m_pipeline = pipeline;
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
    m_device.deferDestroy([device = m_device.native(), pipeline = m_pipeline] { device.destroyPipeline(pipeline); });
}

void VulkanGraphicsPipeline::setDebugName(const std::string& name) {
    m_device.setObjectName(m_pipeline, name);
}


VulkanComputePipeline::VulkanComputePipeline(VulkanDevice& device, const ComputePipelineDesc& desc)
    : m_device(device), m_layout(requireLayout(desc.layout)) {
    if (desc.compute_shader == nullptr)
        throw std::runtime_error("VulkanComputePipeline: a compute shader is required");

    vk::ComputePipelineCreateInfo info({}, stageInfo(*desc.compute_shader), m_layout.native());
    auto [result, pipeline] = m_device.native().createComputePipeline(nullptr, info);
    if (result != vk::Result::eSuccess)
        throw std::runtime_error("VulkanComputePipeline: pipeline creation failed");
    m_pipeline = pipeline;
}

VulkanComputePipeline::~VulkanComputePipeline() {
    m_device.deferDestroy([device = m_device.native(), pipeline = m_pipeline] { device.destroyPipeline(pipeline); });
}

void VulkanComputePipeline::setDebugName(const std::string& name) {
    m_device.setObjectName(m_pipeline, name);
}
