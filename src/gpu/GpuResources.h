#pragma once

#include <cstdint>
#include <string>

#include "GpuDescriptors.h"


class IResource {
public:
    virtual ~IResource() = default;

    virtual void setDebugName(const std::string& name) = 0;
};

class IBuffer : public IResource {
public:
    [[nodiscard]] virtual uint64_t size() const = 0;
    [[nodiscard]] virtual BufferUsage usage() const = 0;
    // Only valid for CpuToGpu / GpuToCpu buffers; returns nullptr otherwise.
    virtual void* map() = 0;
    virtual void unmap() = 0;
    virtual void update(const void* data, uint64_t size, uint64_t offset) = 0;
};

class ITexture : public IResource {
public:
    [[nodiscard]] virtual const TextureDesc& desc() const = 0;
    [[nodiscard]] virtual TextureFormat format() const = 0;
    [[nodiscard]] virtual Extent3D extent() const = 0;
    [[nodiscard]] virtual uint32_t mipLevels() const = 0;
};

class ISampler : public IResource {
public:
    [[nodiscard]] virtual const SamplerDesc& desc() const = 0;
};

class IShaderModule : public IResource {
public:
    [[nodiscard]] virtual ShaderStage stage() const = 0;
    [[nodiscard]] virtual const std::string& entryPoint() const = 0;
};

class IBindGroupLayout : public IResource {
public:
    [[nodiscard]] virtual const BindGroupLayoutDesc& desc() const = 0;
};

class IBindGroup : public IResource {
public:
    [[nodiscard]] virtual IBindGroupLayout& layout() = 0;
};

class IPipelineLayout : public IResource {
public:
    [[nodiscard]] virtual uint32_t bindGroupLayoutCount() const = 0;
};

class IGraphicsPipeline : public IResource {
public:
    [[nodiscard]] virtual IPipelineLayout& layout() = 0;
};

class IComputePipeline : public IResource {
public:
    [[nodiscard]] virtual IPipelineLayout& layout() = 0;
};

class IFence : public IResource {
public:
    // Returns false if the timeout elapsed before the fence was signaled.
    virtual bool wait(uint64_t timeout_ns) = 0;
    virtual void reset() = 0;
    [[nodiscard]] virtual bool isSignaled() const = 0;
};
