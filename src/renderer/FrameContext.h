#pragma once

#include <memory>

#include "ICommandBuffer.h"


// Per-frame-in-flight state, so the CPU can record frame N+1 while the GPU still renders frame N.
struct FrameContext {
    std::unique_ptr<ICommandBuffer> command_buffer;
    std::unique_ptr<IFence> in_flight_fence;
    // Not used by the triangle yet; reserved for per-frame uniforms.
    std::unique_ptr<IBuffer> uniform_buffer;
    std::unique_ptr<IBindGroup> frame_bind_group;
};
