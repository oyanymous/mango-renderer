#pragma once

#include <cstdint>
#include <string>

#include "GpuResources.h"


class ICommandBuffer : public IResource {
public:
    virtual void begin() = 0;
    virtual void end() = 0;

    virtual void beginRenderPass(const RenderPassDesc& desc) = 0;
    virtual void endRenderPass() = 0;
    virtual void setGraphicsPipeline(IGraphicsPipeline& pipeline) = 0;
    virtual void setViewport(const Viewport& viewport) = 0;
    virtual void setScissor(const ScissorRect& rect) = 0;
    virtual void setVertexBuffer(uint32_t slot, IBuffer& buffer, uint64_t offset) = 0;
    virtual void setIndexBuffer(IBuffer& buffer, IndexType type, uint64_t offset) = 0;
    virtual void setBindGroup(uint32_t index, IBindGroup& group) = 0;
    virtual void pushConstants(ShaderStage stages, const void* data, uint32_t size) = 0;
    virtual void draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) = 0;
    virtual void drawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) = 0;

    virtual void beginComputePass() = 0;
    virtual void endComputePass() = 0;
    virtual void setComputePipeline(IComputePipeline& pipeline) = 0;
    virtual void dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;

    virtual void copyBuffer(IBuffer& src, IBuffer& dst, const BufferCopyRegion& region) = 0;
    virtual void copyBufferToTexture(IBuffer& src, ITexture& dst, const TextureCopyRegion& region) = 0;

    virtual void pushDebugGroup(const std::string& label) = 0;
    virtual void popDebugGroup() = 0;
};
