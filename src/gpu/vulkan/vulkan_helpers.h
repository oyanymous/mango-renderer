#pragma once

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan.hpp>
#else
import vulkan_hpp;
#endif

#include <vk_mem_alloc.h>

// <windows.h> (pulled in by VK_USE_PLATFORM_WIN32_KHR) defines DeviceCapabilities as a macro.
#ifdef DeviceCapabilities
#undef DeviceCapabilities
#endif

#include "GpuTypes.h"


namespace vkutil {

vk::Format toVk(TextureFormat format);
TextureFormat fromVk(vk::Format format);
vk::ImageAspectFlags aspectMask(TextureFormat format);
uint32_t bytesPerPixel(TextureFormat format);

vk::BufferUsageFlags toVk(BufferUsage usage);
vk::ImageUsageFlags toVk(TextureUsage usage);
vk::ShaderStageFlags toVk(ShaderStage stage);
vk::DescriptorType toVk(BindingType type);
vk::PrimitiveTopology toVk(PrimitiveTopology topology);
vk::IndexType toVk(IndexType type);
vk::AttachmentLoadOp toVk(LoadOp op);
vk::AttachmentStoreOp toVk(StoreOp op);
vk::CompareOp toVk(CompareOp op);
vk::CullModeFlags toVk(CullMode mode);
vk::FrontFace toVk(FrontFace face);
vk::PolygonMode toVk(PolygonMode mode);
vk::Filter toVk(Filter filter);
vk::SamplerMipmapMode toVkMipmapMode(Filter filter);
vk::SamplerAddressMode toVk(AddressMode mode);
vk::VertexInputRate toVk(VertexStepMode mode);
vk::Format toVk(VertexFormat format);
vk::BlendFactor toVk(BlendFactor factor);
vk::BlendOp toVk(BlendOp op);
vk::ColorComponentFlags toVk(ColorWriteMask mask);
vk::SampleCountFlagBits toVkSampleCount(uint32_t count);

}


struct NativeSurfaceHandle;

namespace vkutil {

// Creates a VkSurfaceKHR for the current platform from a window's native handles.
vk::SurfaceKHR createSurface(vk::Instance instance, const NativeSurfaceHandle& handle);

// Instance extension that `createSurface` needs on this platform.
const char* platformSurfaceExtension();

}
