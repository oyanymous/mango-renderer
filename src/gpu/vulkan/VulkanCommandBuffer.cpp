#include "VulkanCommandBuffer.h"

#include <algorithm>
#include <stdexcept>

#include "VulkanBindGroup.h"
#include "VulkanBuffer.h"
#include "VulkanCommandQueue.h"
#include "VulkanDevice.h"
#include "VulkanPipeline.h"
#include "VulkanTexture.h"


namespace {

struct LayoutSync {
    vk::PipelineStageFlags2 stage;
    vk::AccessFlags2 access;
};

LayoutSync syncFor(vk::ImageLayout layout, bool as_source) {
    using Stage = vk::PipelineStageFlagBits2;
    using Access = vk::AccessFlagBits2;

    switch (layout) {
    case vk::ImageLayout::eUndefined:
        // Swapchain images arrive via a semaphore waited at color-attachment output; starting the
        // barrier there chains the two together.
        return {Stage::eColorAttachmentOutput, {}};
    case vk::ImageLayout::ePresentSrcKHR:
        return as_source ? LayoutSync{Stage::eColorAttachmentOutput, {}} : LayoutSync{Stage::eNone, {}};
    case vk::ImageLayout::eColorAttachmentOptimal:
        return {Stage::eColorAttachmentOutput, Access::eColorAttachmentRead | Access::eColorAttachmentWrite};
    case vk::ImageLayout::eDepthAttachmentOptimal:
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        return {Stage::eEarlyFragmentTests | Stage::eLateFragmentTests,
                Access::eDepthStencilAttachmentRead | Access::eDepthStencilAttachmentWrite};
    case vk::ImageLayout::eShaderReadOnlyOptimal:
        return {Stage::eAllCommands, Access::eShaderRead};
    case vk::ImageLayout::eTransferDstOptimal:
        return {Stage::eAllTransfer, Access::eTransferWrite};
    case vk::ImageLayout::eTransferSrcOptimal:
        return {Stage::eAllTransfer, Access::eTransferRead};
    default:
        return {Stage::eAllCommands, Access::eMemoryRead | Access::eMemoryWrite};
    }
}

vk::ImageLayout attachmentLayout(const ITexture& texture) {
    if (hasStencil(texture.format()))
        return vk::ImageLayout::eDepthStencilAttachmentOptimal;
    if (isDepthFormat(texture.format()))
        return vk::ImageLayout::eDepthAttachmentOptimal;
    return vk::ImageLayout::eColorAttachmentOptimal;
}

}


void ResourceStateTracker::requireLayout(vk::CommandBuffer cmd, VulkanTexture& texture, vk::ImageLayout layout) {
    const vk::ImageLayout current = texture.layout();
    m_layouts[&texture] = layout;
    if (current == layout)
        return;

    const LayoutSync src = syncFor(current, true);
    const LayoutSync dst = syncFor(layout, false);
    vk::ImageMemoryBarrier2 barrier(
        src.stage, src.access,
        dst.stage, dst.access,
        current, layout,
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
        texture.image(), texture.fullRange());
    cmd.pipelineBarrier2(vk::DependencyInfo({}, {}, {}, barrier));

    texture.setLayout(layout);
}


VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice& device, vk::CommandBuffer command_buffer)
    : m_device(device), m_command_buffer(command_buffer) {
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
    m_device.deferDestroy([device = m_device.native(), pool = m_device.queue().commandPool(), cmd = m_command_buffer] {
        device.freeCommandBuffers(pool, cmd);
    });
}

void VulkanCommandBuffer::setDebugName(const std::string& name) {
    m_device.setObjectName(m_command_buffer, name);
}

void VulkanCommandBuffer::begin() {
    m_command_buffer.reset();
    m_command_buffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    m_state_tracker.reset();
    m_swapchains.clear();
    m_current_layout = nullptr;
    m_bind_point = vk::PipelineBindPoint::eGraphics;
}

void VulkanCommandBuffer::end() {
    for (const auto& [texture, layout] : m_state_tracker.layouts()) {
        if (texture->swapchain())
            transitionForPresent(*texture);
    }
    m_command_buffer.end();
}

void VulkanCommandBuffer::beginRenderPass(const RenderPassDesc& desc) {
    transitionForRendering(desc);

    m_render_pass_labeled = !desc.label.empty();
    if (m_render_pass_labeled)
        pushDebugGroup(desc.label);

    std::vector<vk::RenderingAttachmentInfo> color_attachments;
    for (const ColorAttachment& attachment : desc.color_attachments) {
        auto* texture = static_cast<VulkanTexture*>(attachment.texture);
        vk::RenderingAttachmentInfo info(texture->view(), vk::ImageLayout::eColorAttachmentOptimal);
        if (attachment.resolve_texture) {
            info.resolveMode = vk::ResolveModeFlagBits::eAverage;
            info.resolveImageView = static_cast<VulkanTexture*>(attachment.resolve_texture)->view();
            info.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        }
        info.loadOp = vkutil::toVk(attachment.load_op);
        info.storeOp = vkutil::toVk(attachment.store_op);
        const ClearColor& c = attachment.clear_color;
        info.clearValue.color = vk::ClearColorValue(std::array<float, 4>{c.r, c.g, c.b, c.a});
        color_attachments.push_back(info);
    }

    vk::RenderingAttachmentInfo depth_info;
    vk::RenderingAttachmentInfo stencil_info;
    const ITexture* extent_source = desc.color_attachments.empty() ? nullptr : desc.color_attachments.front().texture;
    if (desc.depth_attachment) {
        const DepthAttachment& attachment = *desc.depth_attachment;
        auto* texture = static_cast<VulkanTexture*>(attachment.texture);
        depth_info = vk::RenderingAttachmentInfo(texture->view(), attachmentLayout(*texture));
        depth_info.loadOp = vkutil::toVk(attachment.load_op);
        depth_info.storeOp = vkutil::toVk(attachment.store_op);
        depth_info.clearValue.depthStencil = vk::ClearDepthStencilValue(attachment.clear_depth, attachment.clear_stencil);
        if (hasStencil(texture->format()))
            stencil_info = depth_info;
        if (!extent_source)
            extent_source = texture;
    }
    if (!extent_source)
        throw std::runtime_error("VulkanCommandBuffer: a render pass needs at least one attachment");

    const Extent3D extent = extent_source->extent();
    vk::RenderingInfo rendering_info({}, vk::Rect2D({0, 0}, {extent.width, extent.height}), 1, 0, color_attachments);
    if (desc.depth_attachment)
        rendering_info.setPDepthAttachment(&depth_info);
    if (stencil_info.imageView)
        rendering_info.setPStencilAttachment(&stencil_info);

    m_command_buffer.beginRendering(rendering_info);
    m_bind_point = vk::PipelineBindPoint::eGraphics;
}

void VulkanCommandBuffer::endRenderPass() {
    m_command_buffer.endRendering();
    if (m_render_pass_labeled)
        popDebugGroup();
    m_render_pass_labeled = false;
}

void VulkanCommandBuffer::setGraphicsPipeline(IGraphicsPipeline& pipeline) {
    auto& vk_pipeline = static_cast<VulkanGraphicsPipeline&>(pipeline);
    m_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vk_pipeline.native());
    m_current_layout = &vk_pipeline.vkLayout();
    m_bind_point = vk::PipelineBindPoint::eGraphics;
}

void VulkanCommandBuffer::setViewport(const Viewport& viewport) {
    m_command_buffer.setViewport(0, vk::Viewport(viewport.x, viewport.y, viewport.width, viewport.height,
                                                 viewport.min_depth, viewport.max_depth));
}

void VulkanCommandBuffer::setScissor(const ScissorRect& rect) {
    m_command_buffer.setScissor(0, vk::Rect2D({rect.x, rect.y}, {rect.width, rect.height}));
}

void VulkanCommandBuffer::setVertexBuffer(uint32_t slot, IBuffer& buffer, uint64_t offset) {
    vk::Buffer vk_buffer = static_cast<VulkanBuffer&>(buffer).native();
    m_command_buffer.bindVertexBuffers(slot, vk_buffer, offset);
}

void VulkanCommandBuffer::setIndexBuffer(IBuffer& buffer, IndexType type, uint64_t offset) {
    m_command_buffer.bindIndexBuffer(static_cast<VulkanBuffer&>(buffer).native(), offset, vkutil::toVk(type));
}

void VulkanCommandBuffer::setBindGroup(uint32_t index, IBindGroup& group) {
    if (!m_current_layout)
        throw std::runtime_error("VulkanCommandBuffer: set a pipeline before binding a bind group");
    vk::DescriptorSet set = static_cast<VulkanBindGroup&>(group).native();
    m_command_buffer.bindDescriptorSets(m_bind_point, m_current_layout->native(), index, set, {});
}

void VulkanCommandBuffer::pushConstants(ShaderStage stages, const void* data, uint32_t size) {
    if (!m_current_layout)
        throw std::runtime_error("VulkanCommandBuffer: set a pipeline before pushing constants");
    m_command_buffer.pushConstants(m_current_layout->native(), vkutil::toVk(stages), 0, size, data);
}

void VulkanCommandBuffer::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {
    m_command_buffer.draw(vertex_count, instance_count, first_vertex, first_instance);
}

void VulkanCommandBuffer::drawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) {
    m_command_buffer.drawIndexed(index_count, instance_count, first_index, vertex_offset, first_instance);
}

void VulkanCommandBuffer::beginComputePass() {
    m_bind_point = vk::PipelineBindPoint::eCompute;
}

void VulkanCommandBuffer::endComputePass() {
    memoryBarrier();
    m_bind_point = vk::PipelineBindPoint::eGraphics;
}

void VulkanCommandBuffer::setComputePipeline(IComputePipeline& pipeline) {
    auto& vk_pipeline = static_cast<VulkanComputePipeline&>(pipeline);
    m_command_buffer.bindPipeline(vk::PipelineBindPoint::eCompute, vk_pipeline.native());
    m_current_layout = &vk_pipeline.vkLayout();
    m_bind_point = vk::PipelineBindPoint::eCompute;
}

void VulkanCommandBuffer::dispatch(uint32_t x, uint32_t y, uint32_t z) {
    m_command_buffer.dispatch(x, y, z);
}

void VulkanCommandBuffer::copyBuffer(IBuffer& src, IBuffer& dst, const BufferCopyRegion& region) {
    m_command_buffer.copyBuffer(static_cast<VulkanBuffer&>(src).native(), static_cast<VulkanBuffer&>(dst).native(),
                                vk::BufferCopy(region.src_offset, region.dst_offset, region.size));
    memoryBarrier();
}

void VulkanCommandBuffer::copyBufferToTexture(IBuffer& src, ITexture& dst, const TextureCopyRegion& region) {
    auto& texture = static_cast<VulkanTexture&>(dst);
    m_state_tracker.requireLayout(m_command_buffer, texture, vk::ImageLayout::eTransferDstOptimal);

    const uint32_t bytes_per_pixel = vkutil::bytesPerPixel(texture.format());
    vk::BufferImageCopy copy(
        region.buffer_offset,
        region.bytes_per_row && bytes_per_pixel ? region.bytes_per_row / bytes_per_pixel : 0,
        0,
        vk::ImageSubresourceLayers(vkutil::aspectMask(texture.format()), region.mip_level, region.array_layer, 1),
        vk::Offset3D(region.origin.x, region.origin.y, region.origin.z),
        vk::Extent3D(region.extent.width, region.extent.height, region.extent.depth));
    m_command_buffer.copyBufferToImage(static_cast<VulkanBuffer&>(src).native(), texture.image(),
                                       vk::ImageLayout::eTransferDstOptimal, copy);

    if (hasFlag(texture.desc().usage, TextureUsage::Sampled))
        m_state_tracker.requireLayout(m_command_buffer, texture, vk::ImageLayout::eShaderReadOnlyOptimal);
}

void VulkanCommandBuffer::pushDebugGroup(const std::string& label) {
    if (m_device.debugUtilsEnabled())
        m_command_buffer.beginDebugUtilsLabelEXT(vk::DebugUtilsLabelEXT(label.c_str()));
}

void VulkanCommandBuffer::popDebugGroup() {
    if (m_device.debugUtilsEnabled())
        m_command_buffer.endDebugUtilsLabelEXT();
}

void VulkanCommandBuffer::transitionForRendering(const RenderPassDesc& desc) {
    auto track = [this](ITexture* texture) {
        if (!texture)
            return;
        auto* vk_texture = static_cast<VulkanTexture*>(texture);
        m_state_tracker.requireLayout(m_command_buffer, *vk_texture, attachmentLayout(*vk_texture));
        VulkanSwapchain* swapchain = vk_texture->swapchain();
        if (swapchain && std::find(m_swapchains.begin(), m_swapchains.end(), swapchain) == m_swapchains.end())
            m_swapchains.push_back(swapchain);
    };

    for (const ColorAttachment& attachment : desc.color_attachments) {
        track(attachment.texture);
        track(attachment.resolve_texture);
    }
    if (desc.depth_attachment)
        track(desc.depth_attachment->texture);
}

void VulkanCommandBuffer::transitionForPresent(VulkanTexture& image) {
    m_state_tracker.requireLayout(m_command_buffer, image, vk::ImageLayout::ePresentSrcKHR);
}

void VulkanCommandBuffer::memoryBarrier() {
    vk::MemoryBarrier2 barrier(
        vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
        vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite);
    m_command_buffer.pipelineBarrier2(vk::DependencyInfo({}, barrier));
}
