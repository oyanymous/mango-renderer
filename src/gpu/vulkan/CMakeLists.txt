include(VulkanDeps)

add_library(GpuVulkan
    vulkan_helpers.h
    vulkan_helpers.cpp
    vma_implementation.cpp
    VulkanDevice.h
    VulkanDevice.cpp
    VulkanCommandQueue.h
    VulkanCommandQueue.cpp
    VulkanCommandBuffer.h
    VulkanCommandBuffer.cpp
    VulkanSwapchain.h
    VulkanSwapchain.cpp
    VulkanBuffer.h
    VulkanBuffer.cpp
    VulkanTexture.h
    VulkanTexture.cpp
    VulkanShaderModule.h
    VulkanShaderModule.cpp
    VulkanBindGroup.h
    VulkanBindGroup.cpp
    VulkanPipeline.h
    VulkanPipeline.cpp
    VulkanFence.h
    VulkanFence.cpp
)

target_link_libraries(GpuVulkan PUBLIC GpuApi Vulkan::Headers VulkanMemoryAllocator::Headers)

target_compile_definitions(GpuVulkan PUBLIC
    VK_NO_PROTOTYPES
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VMA_STATIC_VULKAN_FUNCTIONS=0
    VMA_DYNAMIC_VULKAN_FUNCTIONS=1
)

# The platform defines change the layout of Vulkan-Hpp's dispatcher, so they must be
# identical in every translation unit that includes vulkan.hpp.
if (WIN32)
    target_compile_definitions(GpuVulkan PUBLIC VK_USE_PLATFORM_WIN32_KHR WIN32_LEAN_AND_MEAN NOMINMAX)
elseif (APPLE)
    target_compile_definitions(GpuVulkan PUBLIC VK_USE_PLATFORM_METAL_EXT)
endif ()

target_compile_features(GpuVulkan PUBLIC cxx_std_20)

target_include_directories(GpuVulkan
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
