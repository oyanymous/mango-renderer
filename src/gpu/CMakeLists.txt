option(MANGO_RHI_VULKAN "Build the Vulkan graphics backend" ON)
option(MANGO_RHI_METAL "Build the Metal graphics backend" OFF)
option(MANGO_RHI_OPENGL "Build the OpenGL graphics backend" OFF)

# API-agnostic interfaces, descriptors and enums shared by the renderer and every backend.
add_library(GpuApi INTERFACE
    GpuTypes.h
    GpuDescriptors.h
    GpuResources.h
    ICommandBuffer.h
    ICommandQueue.h
    ISwapchain.h
    IGraphicsDevice.h
)

target_link_libraries(GpuApi INTERFACE Platform)
target_compile_features(GpuApi INTERFACE cxx_std_20)

target_include_directories(GpuApi INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

if (MANGO_RHI_VULKAN)
    add_subdirectory(vulkan)
endif ()

# Runtime backend selection; the only target the renderer needs to link.
add_library(Gpu
    GraphicsDeviceFactory.h
    GraphicsDeviceFactory.cpp
)

target_link_libraries(Gpu PUBLIC GpuApi)

target_compile_definitions(Gpu PRIVATE
    MANGO_RHI_VULKAN=$<BOOL:${MANGO_RHI_VULKAN}>
    MANGO_RHI_METAL=$<BOOL:${MANGO_RHI_METAL}>
    MANGO_RHI_OPENGL=$<BOOL:${MANGO_RHI_OPENGL}>
)

if (MANGO_RHI_VULKAN)
    target_link_libraries(Gpu PRIVATE GpuVulkan)
endif ()
