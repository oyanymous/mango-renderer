#pragma once

#include <memory>

#include "GpuResources.h"
#include "ICommandQueue.h"
#include "ISwapchain.h"


class IGraphicsDevice {
public:
    virtual ~IGraphicsDevice() = default;

    [[nodiscard]] virtual GraphicsAPI api() const = 0;
    [[nodiscard]] virtual const DeviceCapabilities& capabilities() const = 0;
    virtual ICommandQueue& graphicsQueue() = 0;

    virtual std::unique_ptr<ISwapchain> createSwapchain(const SwapchainDesc& desc) = 0;
    virtual std::unique_ptr<IBuffer> createBuffer(const BufferDesc& desc, const void* initial_data = nullptr) = 0;
    virtual std::unique_ptr<ITexture> createTexture(const TextureDesc& desc) = 0;
    virtual std::unique_ptr<ISampler> createSampler(const SamplerDesc& desc) = 0;
    virtual std::unique_ptr<IShaderModule> createShaderModule(const ShaderModuleDesc& desc) = 0;
    virtual std::unique_ptr<IBindGroupLayout> createBindGroupLayout(const BindGroupLayoutDesc& desc) = 0;
    virtual std::unique_ptr<IBindGroup> createBindGroup(const BindGroupDesc& desc) = 0;
    virtual std::unique_ptr<IPipelineLayout> createPipelineLayout(const PipelineLayoutDesc& desc) = 0;
    virtual std::unique_ptr<IGraphicsPipeline> createGraphicsPipeline(const GraphicsPipelineDesc& desc) = 0;
    virtual std::unique_ptr<IComputePipeline> createComputePipeline(const ComputePipelineDesc& desc) = 0;
    virtual std::unique_ptr<IFence> createFence(bool signaled) = 0;

    virtual void waitIdle() = 0;
};
