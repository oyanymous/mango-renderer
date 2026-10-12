#include "GraphicsDeviceFactory.h"

#include <stdexcept>

#if MANGO_RHI_VULKAN
#include "VulkanDevice.h"
#endif


std::unique_ptr<IGraphicsDevice> GraphicsDeviceFactory::create(const DeviceDesc& desc) {
    DeviceDesc resolved = desc;
    if (resolved.api == GraphicsAPI::Auto)
        resolved.api = defaultAPI();

    switch (resolved.api) {
#if MANGO_RHI_VULKAN
    case GraphicsAPI::Vulkan:
        return std::make_unique<VulkanDevice>(resolved);
#endif
    default:
        throw std::runtime_error("GraphicsDeviceFactory: requested graphics API is not available in this build");
    }
}

bool GraphicsDeviceFactory::isSupported(GraphicsAPI api) {
    switch (api) {
    case GraphicsAPI::Auto:
        return isSupported(defaultAPI());
    case GraphicsAPI::Metal:
        return MANGO_RHI_METAL;
    case GraphicsAPI::Vulkan:
        return MANGO_RHI_VULKAN;
    case GraphicsAPI::OpenGL:
        return MANGO_RHI_OPENGL;
    }
    return false;
}

GraphicsAPI GraphicsDeviceFactory::defaultAPI() {
#if defined(__APPLE__)
    if (MANGO_RHI_METAL)
        return GraphicsAPI::Metal;
#endif
    if (MANGO_RHI_VULKAN)
        return GraphicsAPI::Vulkan;
    if (MANGO_RHI_METAL)
        return GraphicsAPI::Metal;
    return GraphicsAPI::OpenGL;
}
