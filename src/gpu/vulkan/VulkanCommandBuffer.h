#pragma once

#include <unordered_map>
#include <vector>

#include "ICommandBuffer.h"
#include "vulkan_helpers.h"

class VulkanDevice;
class VulkanPipelineLayout;
class VulkanSwapchain;
class VulkanTexture;


// Inserts the image layout transitions that Metal and OpenGL perform implicitly.
class ResourceStateTracker {
public:
    void requireLayout(vk::CommandBuffer cmd, VulkanTexture& texture, vk::ImageLayout layout);
    void reset() { m_layouts.clear(); }

    // Every texture touched since the last reset, with its layout at the end of the recording.
    [[nodiscard]] const std::unordered_map<VulkanTexture*, vk::ImageLayout>& layouts() const { return m_layouts; }
private:
    std::unordered_map<VulkanTexture*, vk::ImageLayout> m_layouts;
};


class VulkanCommandBuffer final : public ICommandBuffer {
public:
    VulkanCommandBuffer(VulkanDevice& device, vk::CommandBuffer command_buffer);
    ~VulkanCommandBuffer() override;

    void setDebugName(const std::string& name) override;

    void begin() override;
    void end() override;

    void beginRenderPass(const RenderPassDesc& desc) override;
    void endRenderPass() override;
    void setGraphicsPipeline(IGraphicsPipeline& pipeline) override;
    void setViewport(const Viewport& viewport) override;
    void setScissor(const ScissorRect& rect) override;
    void setVertexBuffer(uint32_t slot, IBuffer& buffer, uint64_t offset) override;
    void setIndexBuffer(IBuffer& buffer, IndexType type, uint64_t offset) override;
    void setBindGroup(uint32_t index, IBindGroup& group) override;
    void pushConstants(ShaderStage stages, const void* data, uint32_t size) override;
    void draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override;
    void drawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) override;

    void beginComputePass() override;
    void endComputePass() override;
    void setComputePipeline(IComputePipeline& pipeline) override;
    void dispatch(uint32_t x, uint32_t y, uint32_t z) override;

    void copyBuffer(IBuffer& src, IBuffer& dst, const BufferCopyRegion& region) override;
    void copyBufferToTexture(IBuffer& src, ITexture& dst, const TextureCopyRegion& region) override;

    void pushDebugGroup(const std::string& label) override;
    void popDebugGroup() override;

    [[nodiscard]] vk::CommandBuffer native() const { return m_command_buffer; }
    // Swapchains whose current image this command buffer renders to.
    [[nodiscard]] const std::vector<VulkanSwapchain*>& swapchains() const { return m_swapchains; }
private:
    void transitionForRendering(const RenderPassDesc& desc);
    void transitionForPresent(VulkanTexture& image);
    // Makes all prior writes visible to all later commands (buffers are not tracked individually).
    void memoryBarrier();

    VulkanDevice& m_device;
    vk::CommandBuffer m_command_buffer;
    VulkanPipelineLayout* m_current_layout = nullptr;
    vk::PipelineBindPoint m_bind_point = vk::PipelineBindPoint::eGraphics;
    ResourceStateTracker m_state_tracker;
    std::vector<VulkanSwapchain*> m_swapchains;
    bool m_render_pass_labeled = false;
};
