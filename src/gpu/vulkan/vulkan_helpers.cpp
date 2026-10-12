#include "vulkan_helpers.h"

#include "NativeSurfaceHandle.h"

// Storage for Vulkan-Hpp's default dynamic dispatcher; the Vulkan loader is
// resolved at runtime, so nothing links against vulkan-1.lib / libvulkan.
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE


namespace vkutil {

vk::Format toVk(TextureFormat format) {
    switch (format) {
    case TextureFormat::Undefined: return vk::Format::eUndefined;
    case TextureFormat::R8Unorm: return vk::Format::eR8Unorm;
    case TextureFormat::RG8Unorm: return vk::Format::eR8G8Unorm;
    case TextureFormat::RGBA8Unorm: return vk::Format::eR8G8B8A8Unorm;
    case TextureFormat::RGBA8Srgb: return vk::Format::eR8G8B8A8Srgb;
    case TextureFormat::BGRA8Unorm: return vk::Format::eB8G8R8A8Unorm;
    case TextureFormat::BGRA8Srgb: return vk::Format::eB8G8R8A8Srgb;
    case TextureFormat::R16Float: return vk::Format::eR16Sfloat;
    case TextureFormat::RGBA16Float: return vk::Format::eR16G16B16A16Sfloat;
    case TextureFormat::R32Float: return vk::Format::eR32Sfloat;
    case TextureFormat::RGBA32Float: return vk::Format::eR32G32B32A32Sfloat;
    case TextureFormat::Depth32Float: return vk::Format::eD32Sfloat;
    case TextureFormat::Depth24Stencil8: return vk::Format::eD24UnormS8Uint;
    }
    return vk::Format::eUndefined;
}

TextureFormat fromVk(vk::Format format) {
    switch (format) {
    case vk::Format::eR8Unorm: return TextureFormat::R8Unorm;
    case vk::Format::eR8G8Unorm: return TextureFormat::RG8Unorm;
    case vk::Format::eR8G8B8A8Unorm: return TextureFormat::RGBA8Unorm;
    case vk::Format::eR8G8B8A8Srgb: return TextureFormat::RGBA8Srgb;
    case vk::Format::eB8G8R8A8Unorm: return TextureFormat::BGRA8Unorm;
    case vk::Format::eB8G8R8A8Srgb: return TextureFormat::BGRA8Srgb;
    case vk::Format::eR16Sfloat: return TextureFormat::R16Float;
    case vk::Format::eR16G16B16A16Sfloat: return TextureFormat::RGBA16Float;
    case vk::Format::eR32Sfloat: return TextureFormat::R32Float;
    case vk::Format::eR32G32B32A32Sfloat: return TextureFormat::RGBA32Float;
    case vk::Format::eD32Sfloat: return TextureFormat::Depth32Float;
    case vk::Format::eD24UnormS8Uint: return TextureFormat::Depth24Stencil8;
    default: return TextureFormat::Undefined;
    }
}

vk::ImageAspectFlags aspectMask(TextureFormat format) {
    if (hasStencil(format))
        return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    if (isDepthFormat(format))
        return vk::ImageAspectFlagBits::eDepth;
    return vk::ImageAspectFlagBits::eColor;
}

uint32_t bytesPerPixel(TextureFormat format) {
    switch (format) {
    case TextureFormat::R8Unorm: return 1;
    case TextureFormat::RG8Unorm:
    case TextureFormat::R16Float: return 2;
    case TextureFormat::RGBA8Unorm:
    case TextureFormat::RGBA8Srgb:
    case TextureFormat::BGRA8Unorm:
    case TextureFormat::BGRA8Srgb:
    case TextureFormat::R32Float:
    case TextureFormat::Depth32Float:
    case TextureFormat::Depth24Stencil8: return 4;
    case TextureFormat::RGBA16Float: return 8;
    case TextureFormat::RGBA32Float: return 16;
    case TextureFormat::Undefined: return 0;
    }
    return 0;
}

vk::BufferUsageFlags toVk(BufferUsage usage) {
    vk::BufferUsageFlags flags;
    if (hasFlag(usage, BufferUsage::Vertex)) flags |= vk::BufferUsageFlagBits::eVertexBuffer;
    if (hasFlag(usage, BufferUsage::Index)) flags |= vk::BufferUsageFlagBits::eIndexBuffer;
    if (hasFlag(usage, BufferUsage::Uniform)) flags |= vk::BufferUsageFlagBits::eUniformBuffer;
    if (hasFlag(usage, BufferUsage::Storage)) flags |= vk::BufferUsageFlagBits::eStorageBuffer;
    if (hasFlag(usage, BufferUsage::Indirect)) flags |= vk::BufferUsageFlagBits::eIndirectBuffer;
    if (hasFlag(usage, BufferUsage::CopySrc)) flags |= vk::BufferUsageFlagBits::eTransferSrc;
    if (hasFlag(usage, BufferUsage::CopyDst)) flags |= vk::BufferUsageFlagBits::eTransferDst;
    return flags;
}

vk::ImageUsageFlags toVk(TextureUsage usage) {
    vk::ImageUsageFlags flags;
    if (hasFlag(usage, TextureUsage::Sampled)) flags |= vk::ImageUsageFlagBits::eSampled;
    if (hasFlag(usage, TextureUsage::Storage)) flags |= vk::ImageUsageFlagBits::eStorage;
    if (hasFlag(usage, TextureUsage::ColorAttachment)) flags |= vk::ImageUsageFlagBits::eColorAttachment;
    if (hasFlag(usage, TextureUsage::DepthStencilAttachment)) flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
    if (hasFlag(usage, TextureUsage::CopySrc)) flags |= vk::ImageUsageFlagBits::eTransferSrc;
    if (hasFlag(usage, TextureUsage::CopyDst)) flags |= vk::ImageUsageFlagBits::eTransferDst;
    return flags;
}

vk::ShaderStageFlags toVk(ShaderStage stage) {
    vk::ShaderStageFlags flags;
    if (hasFlag(stage, ShaderStage::Vertex)) flags |= vk::ShaderStageFlagBits::eVertex;
    if (hasFlag(stage, ShaderStage::Fragment)) flags |= vk::ShaderStageFlagBits::eFragment;
    if (hasFlag(stage, ShaderStage::Compute)) flags |= vk::ShaderStageFlagBits::eCompute;
    return flags;
}

vk::DescriptorType toVk(BindingType type) {
    switch (type) {
    case BindingType::UniformBuffer: return vk::DescriptorType::eUniformBuffer;
    case BindingType::StorageBuffer: return vk::DescriptorType::eStorageBuffer;
    case BindingType::SampledTexture: return vk::DescriptorType::eSampledImage;
    case BindingType::StorageTexture: return vk::DescriptorType::eStorageImage;
    case BindingType::Sampler: return vk::DescriptorType::eSampler;
    case BindingType::CombinedTextureSampler: return vk::DescriptorType::eCombinedImageSampler;
    }
    return vk::DescriptorType::eUniformBuffer;
}

vk::PrimitiveTopology toVk(PrimitiveTopology topology) {
    switch (topology) {
    case PrimitiveTopology::PointList: return vk::PrimitiveTopology::ePointList;
    case PrimitiveTopology::LineList: return vk::PrimitiveTopology::eLineList;
    case PrimitiveTopology::LineStrip: return vk::PrimitiveTopology::eLineStrip;
    case PrimitiveTopology::TriangleList: return vk::PrimitiveTopology::eTriangleList;
    case PrimitiveTopology::TriangleStrip: return vk::PrimitiveTopology::eTriangleStrip;
    }
    return vk::PrimitiveTopology::eTriangleList;
}

vk::IndexType toVk(IndexType type) {
    return type == IndexType::Uint16 ? vk::IndexType::eUint16 : vk::IndexType::eUint32;
}

vk::AttachmentLoadOp toVk(LoadOp op) {
    switch (op) {
    case LoadOp::Load: return vk::AttachmentLoadOp::eLoad;
    case LoadOp::Clear: return vk::AttachmentLoadOp::eClear;
    case LoadOp::DontCare: return vk::AttachmentLoadOp::eDontCare;
    }
    return vk::AttachmentLoadOp::eDontCare;
}

vk::AttachmentStoreOp toVk(StoreOp op) {
    return op == StoreOp::Store ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare;
}

vk::CompareOp toVk(CompareOp op) {
    switch (op) {
    case CompareOp::Never: return vk::CompareOp::eNever;
    case CompareOp::Less: return vk::CompareOp::eLess;
    case CompareOp::Equal: return vk::CompareOp::eEqual;
    case CompareOp::LessEqual: return vk::CompareOp::eLessOrEqual;
    case CompareOp::Greater: return vk::CompareOp::eGreater;
    case CompareOp::NotEqual: return vk::CompareOp::eNotEqual;
    case CompareOp::GreaterEqual: return vk::CompareOp::eGreaterOrEqual;
    case CompareOp::Always: return vk::CompareOp::eAlways;
    }
    return vk::CompareOp::eAlways;
}

vk::CullModeFlags toVk(CullMode mode) {
    switch (mode) {
    case CullMode::None: return vk::CullModeFlagBits::eNone;
    case CullMode::Front: return vk::CullModeFlagBits::eFront;
    case CullMode::Back: return vk::CullModeFlagBits::eBack;
    }
    return vk::CullModeFlagBits::eNone;
}

vk::FrontFace toVk(FrontFace face) {
    return face == FrontFace::Clockwise ? vk::FrontFace::eClockwise : vk::FrontFace::eCounterClockwise;
}

vk::PolygonMode toVk(PolygonMode mode) {
    return mode == PolygonMode::Line ? vk::PolygonMode::eLine : vk::PolygonMode::eFill;
}

vk::Filter toVk(Filter filter) {
    return filter == Filter::Nearest ? vk::Filter::eNearest : vk::Filter::eLinear;
}

vk::SamplerMipmapMode toVkMipmapMode(Filter filter) {
    return filter == Filter::Nearest ? vk::SamplerMipmapMode::eNearest : vk::SamplerMipmapMode::eLinear;
}

vk::SamplerAddressMode toVk(AddressMode mode) {
    switch (mode) {
    case AddressMode::Repeat: return vk::SamplerAddressMode::eRepeat;
    case AddressMode::MirroredRepeat: return vk::SamplerAddressMode::eMirroredRepeat;
    case AddressMode::ClampToEdge: return vk::SamplerAddressMode::eClampToEdge;
    case AddressMode::ClampToBorder: return vk::SamplerAddressMode::eClampToBorder;
    }
    return vk::SamplerAddressMode::eRepeat;
}

vk::VertexInputRate toVk(VertexStepMode mode) {
    return mode == VertexStepMode::Instance ? vk::VertexInputRate::eInstance : vk::VertexInputRate::eVertex;
}

vk::Format toVk(VertexFormat format) {
    switch (format) {
    case VertexFormat::Float: return vk::Format::eR32Sfloat;
    case VertexFormat::Float2: return vk::Format::eR32G32Sfloat;
    case VertexFormat::Float3: return vk::Format::eR32G32B32Sfloat;
    case VertexFormat::Float4: return vk::Format::eR32G32B32A32Sfloat;
    case VertexFormat::UByte4Norm: return vk::Format::eR8G8B8A8Unorm;
    case VertexFormat::UInt: return vk::Format::eR32Uint;
    case VertexFormat::UInt2: return vk::Format::eR32G32Uint;
    case VertexFormat::UInt3: return vk::Format::eR32G32B32Uint;
    case VertexFormat::UInt4: return vk::Format::eR32G32B32A32Uint;
    }
    return vk::Format::eUndefined;
}

vk::BlendFactor toVk(BlendFactor factor) {
    switch (factor) {
    case BlendFactor::Zero: return vk::BlendFactor::eZero;
    case BlendFactor::One: return vk::BlendFactor::eOne;
    case BlendFactor::SrcColor: return vk::BlendFactor::eSrcColor;
    case BlendFactor::OneMinusSrcColor: return vk::BlendFactor::eOneMinusSrcColor;
    case BlendFactor::DstColor: return vk::BlendFactor::eDstColor;
    case BlendFactor::OneMinusDstColor: return vk::BlendFactor::eOneMinusDstColor;
    case BlendFactor::SrcAlpha: return vk::BlendFactor::eSrcAlpha;
    case BlendFactor::OneMinusSrcAlpha: return vk::BlendFactor::eOneMinusSrcAlpha;
    case BlendFactor::DstAlpha: return vk::BlendFactor::eDstAlpha;
    case BlendFactor::OneMinusDstAlpha: return vk::BlendFactor::eOneMinusDstAlpha;
    }
    return vk::BlendFactor::eOne;
}

vk::BlendOp toVk(BlendOp op) {
    switch (op) {
    case BlendOp::Add: return vk::BlendOp::eAdd;
    case BlendOp::Subtract: return vk::BlendOp::eSubtract;
    case BlendOp::ReverseSubtract: return vk::BlendOp::eReverseSubtract;
    case BlendOp::Min: return vk::BlendOp::eMin;
    case BlendOp::Max: return vk::BlendOp::eMax;
    }
    return vk::BlendOp::eAdd;
}

vk::ColorComponentFlags toVk(ColorWriteMask mask) {
    vk::ColorComponentFlags flags;
    if (hasFlag(mask, ColorWriteMask::Red)) flags |= vk::ColorComponentFlagBits::eR;
    if (hasFlag(mask, ColorWriteMask::Green)) flags |= vk::ColorComponentFlagBits::eG;
    if (hasFlag(mask, ColorWriteMask::Blue)) flags |= vk::ColorComponentFlagBits::eB;
    if (hasFlag(mask, ColorWriteMask::Alpha)) flags |= vk::ColorComponentFlagBits::eA;
    return flags;
}

vk::SampleCountFlagBits toVkSampleCount(uint32_t count) {
    switch (count) {
    case 2: return vk::SampleCountFlagBits::e2;
    case 4: return vk::SampleCountFlagBits::e4;
    case 8: return vk::SampleCountFlagBits::e8;
    case 16: return vk::SampleCountFlagBits::e16;
    default: return vk::SampleCountFlagBits::e1;
    }
}

}


namespace vkutil {

vk::SurfaceKHR createSurface(vk::Instance instance, const NativeSurfaceHandle& handle) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    vk::Win32SurfaceCreateInfoKHR info({}, static_cast<HINSTANCE>(handle.display), static_cast<HWND>(handle.window));
    return instance.createWin32SurfaceKHR(info);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    vk::MetalSurfaceCreateInfoEXT info({}, static_cast<const CAMetalLayer*>(handle.layer));
    return instance.createMetalSurfaceEXT(info);
#else
#error "No Vulkan surface support for this platform"
#endif
}

const char* platformSurfaceExtension() {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    return VK_EXT_METAL_SURFACE_EXTENSION_NAME;
#endif
}

}
