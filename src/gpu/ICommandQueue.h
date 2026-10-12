#pragma once

#include <memory>
#include <span>

#include "ICommandBuffer.h"
#include "ISwapchain.h"


class ICommandQueue {
public:
    virtual ~ICommandQueue() = default;

    virtual std::unique_ptr<ICommandBuffer> createCommandBuffer() = 0;
    virtual void submit(std::span<ICommandBuffer* const> buffers, IFence* signal_fence) = 0;
    virtual void present(ISwapchain& swapchain) = 0;
    virtual void waitIdle() = 0;
};
