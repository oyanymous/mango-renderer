#pragma once

#include <cstdint>
#include <type_traits>


// Enables `|`, `&` and `hasFlag` on an `enum class` used as a bit mask.
#define MANGO_ENUM_FLAGS(Enum)                                                                   \
    constexpr Enum operator|(Enum a, Enum b) {                                                   \
        using U = std::underlying_type_t<Enum>;                                                  \
        return static_cast<Enum>(static_cast<U>(a) | static_cast<U>(b));                         \
    }                                                                                            \
    constexpr Enum operator&(Enum a, Enum b) {                                                   \
        using U = std::underlying_type_t<Enum>;                                                  \
        return static_cast<Enum>(static_cast<U>(a) & static_cast<U>(b));                         \
    }                                                                                            \
    constexpr Enum& operator|=(Enum& a, Enum b) { return a = a | b; }                            \
    constexpr bool hasFlag(Enum value, Enum flag) {                                              \
        return static_cast<std::underlying_type_t<Enum>>(value & flag) != 0;                     \
    }


enum class GraphicsAPI { Auto, Metal, Vulkan, OpenGL };

enum class BufferUsage : uint32_t {
    None = 0,
    Vertex = 1 << 0,
    Index = 1 << 1,
    Uniform = 1 << 2,
    Storage = 1 << 3,
    Indirect = 1 << 4,
    CopySrc = 1 << 5,
    CopyDst = 1 << 6,
};
MANGO_ENUM_FLAGS(BufferUsage)

enum class MemoryType { GpuOnly, CpuToGpu, GpuToCpu };

enum class TextureType { Texture1D, Texture2D, Texture3D, TextureCube, Texture2DArray };

enum class TextureFormat {
    Undefined,
    R8Unorm,
    RG8Unorm,
    RGBA8Unorm,
    RGBA8Srgb,
    BGRA8Unorm,
    BGRA8Srgb,
    R16Float,
    RGBA16Float,
    R32Float,
    RGBA32Float,
    Depth32Float,
    Depth24Stencil8,
};

constexpr bool isDepthFormat(TextureFormat format) {
    return format == TextureFormat::Depth32Float || format == TextureFormat::Depth24Stencil8;
}

constexpr bool hasStencil(TextureFormat format) {
    return format == TextureFormat::Depth24Stencil8;
}

enum class TextureUsage : uint32_t {
    None = 0,
    Sampled = 1 << 0,
    Storage = 1 << 1,
    ColorAttachment = 1 << 2,
    DepthStencilAttachment = 1 << 3,
    CopySrc = 1 << 4,
    CopyDst = 1 << 5,
};
MANGO_ENUM_FLAGS(TextureUsage)

enum class ShaderStage : uint32_t {
    None = 0,
    Vertex = 1 << 0,
    Fragment = 1 << 1,
    Compute = 1 << 2,
};
MANGO_ENUM_FLAGS(ShaderStage)

enum class ShaderLanguage { SPIRV, MSL, MetalLib, GLSL };

enum class BindingType {
    UniformBuffer,
    StorageBuffer,
    SampledTexture,
    StorageTexture,
    Sampler,
    CombinedTextureSampler,
};

enum class PrimitiveTopology { PointList, LineList, LineStrip, TriangleList, TriangleStrip };

enum class IndexType { Uint16, Uint32 };

enum class LoadOp { Load, Clear, DontCare };

enum class StoreOp { Store, DontCare };

enum class CompareOp { Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always };

enum class CullMode { None, Front, Back };

enum class FrontFace { CounterClockwise, Clockwise };

enum class PolygonMode { Fill, Line };

enum class ClipSpaceDepth { ZeroToOne, NegativeOneToOne };

enum class Filter { Nearest, Linear };

enum class AddressMode { Repeat, MirroredRepeat, ClampToEdge, ClampToBorder };

enum class VertexStepMode { Vertex, Instance };

enum class VertexFormat {
    Float,
    Float2,
    Float3,
    Float4,
    UByte4Norm,
    UInt,
    UInt2,
    UInt3,
    UInt4,
};

enum class BlendFactor {
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
};

enum class BlendOp { Add, Subtract, ReverseSubtract, Min, Max };

enum class ColorWriteMask : uint32_t {
    None = 0,
    Red = 1 << 0,
    Green = 1 << 1,
    Blue = 1 << 2,
    Alpha = 1 << 3,
    All = Red | Green | Blue | Alpha,
};
MANGO_ENUM_FLAGS(ColorWriteMask)


struct Extent2D {
    uint32_t width = 0;
    uint32_t height = 0;
};

struct Extent3D {
    uint32_t width = 1;
    uint32_t height = 1;
    uint32_t depth = 1;
};

struct Offset3D {
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;
};

// Linear RGBA clear value. (`Color` lives in ui/, which the GPU layer must not depend on.)
struct ClearColor {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};
