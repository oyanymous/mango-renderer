#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "GpuTypes.h"
#include "NativeSurfaceHandle.h"

class IBuffer;
class ITexture;
class ISampler;
class IShaderModule;
class IBindGroupLayout;
class IPipelineLayout;


struct DeviceDesc {
    GraphicsAPI api = GraphicsAPI::Auto;
    NativeSurfaceHandle surface;
    bool enable_validation = false;
    std::string application_name = "Mango";
};

struct DeviceCapabilities {
    std::string device_name;
    bool supports_compute = false;
    bool supports_push_constants = false;
    uint32_t max_texture_size = 0;
    uint32_t max_bind_groups = 0;
    uint32_t max_color_attachments = 0;
    uint32_t uniform_buffer_alignment = 0;
    ClipSpaceDepth clip_depth = ClipSpaceDepth::ZeroToOne;
    bool framebuffer_origin_top_left = true;
};

struct SwapchainDesc {
    NativeSurfaceHandle surface;
    Extent2D extent;
    TextureFormat format = TextureFormat::BGRA8Unorm;
    uint32_t image_count = 3;
    bool vsync = true;
};

struct BufferDesc {
    uint64_t size = 0;
    BufferUsage usage = BufferUsage::None;
    MemoryType memory = MemoryType::GpuOnly;
};

struct TextureDesc {
    TextureType type = TextureType::Texture2D;
    TextureFormat format = TextureFormat::RGBA8Unorm;
    Extent3D extent;
    uint32_t mip_levels = 1;
    uint32_t array_layers = 1;
    uint32_t sample_count = 1;
    TextureUsage usage = TextureUsage::Sampled;
};

struct SamplerDesc {
    Filter min_filter = Filter::Linear;
    Filter mag_filter = Filter::Linear;
    Filter mip_filter = Filter::Linear;
    AddressMode address_u = AddressMode::Repeat;
    AddressMode address_v = AddressMode::Repeat;
    AddressMode address_w = AddressMode::Repeat;
    float max_anisotropy = 1.0f;
    std::optional<CompareOp> compare;
};

struct ShaderSource {
    ShaderLanguage language = ShaderLanguage::SPIRV;
    std::vector<uint8_t> code;
};

struct ShaderModuleDesc {
    ShaderStage stage = ShaderStage::None;
    std::string entry_point = "main";
    std::vector<ShaderSource> sources;
};

struct BindGroupLayoutEntry {
    uint32_t binding = 0;
    BindingType type = BindingType::UniformBuffer;
    ShaderStage visibility = ShaderStage::None;
    uint32_t count = 1;
};

struct BindGroupLayoutDesc {
    std::vector<BindGroupLayoutEntry> entries;
};

struct BindGroupEntry {
    uint32_t binding = 0;
    IBuffer* buffer = nullptr;
    uint64_t offset = 0;
    uint64_t range = 0; // 0 = whole buffer
    ITexture* texture = nullptr;
    ISampler* sampler = nullptr;
};

struct BindGroupDesc {
    IBindGroupLayout* layout = nullptr;
    std::vector<BindGroupEntry> entries;
};

struct PushConstantRange {
    ShaderStage stages = ShaderStage::None;
    uint32_t offset = 0;
    uint32_t size = 0;
};

struct PipelineLayoutDesc {
    std::vector<IBindGroupLayout*> bind_group_layouts;
    std::vector<PushConstantRange> push_constants;
};

struct VertexBufferLayout {
    uint32_t binding = 0;
    uint32_t stride = 0;
    VertexStepMode step_mode = VertexStepMode::Vertex;
};

struct VertexAttribute {
    uint32_t location = 0;
    uint32_t binding = 0;
    VertexFormat format = VertexFormat::Float4;
    uint32_t offset = 0;
};

struct VertexLayout {
    std::vector<VertexBufferLayout> buffers;
    std::vector<VertexAttribute> attributes;
};

struct RasterState {
    CullMode cull_mode = CullMode::None;
    FrontFace front_face = FrontFace::CounterClockwise;
    PolygonMode polygon_mode = PolygonMode::Fill;
    bool depth_clamp = false;
};

struct DepthStencilState {
    bool depth_test = false;
    bool depth_write = false;
    CompareOp depth_compare = CompareOp::Less;
};

struct BlendState {
    bool enabled = false;
    BlendFactor src_color = BlendFactor::One;
    BlendFactor dst_color = BlendFactor::Zero;
    BlendOp color_op = BlendOp::Add;
    BlendFactor src_alpha = BlendFactor::One;
    BlendFactor dst_alpha = BlendFactor::Zero;
    BlendOp alpha_op = BlendOp::Add;
    ColorWriteMask write_mask = ColorWriteMask::All;
};

struct GraphicsPipelineDesc {
    IShaderModule* vertex_shader = nullptr;
    IShaderModule* fragment_shader = nullptr;
    IPipelineLayout* layout = nullptr;
    VertexLayout vertex_layout;
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    RasterState raster;
    DepthStencilState depth_stencil;
    std::vector<BlendState> blend; // one per color format; missing entries use the default
    std::vector<TextureFormat> color_formats;
    TextureFormat depth_format = TextureFormat::Undefined;
    uint32_t sample_count = 1;
};

struct ComputePipelineDesc {
    IShaderModule* compute_shader = nullptr;
    IPipelineLayout* layout = nullptr;
};

struct ColorAttachment {
    ITexture* texture = nullptr;
    ITexture* resolve_texture = nullptr;
    LoadOp load_op = LoadOp::Clear;
    StoreOp store_op = StoreOp::Store;
    ClearColor clear_color;
};

struct DepthAttachment {
    ITexture* texture = nullptr;
    LoadOp load_op = LoadOp::Clear;
    StoreOp store_op = StoreOp::DontCare;
    float clear_depth = 1.0f;
    uint8_t clear_stencil = 0;
};

struct RenderPassDesc {
    std::vector<ColorAttachment> color_attachments;
    std::optional<DepthAttachment> depth_attachment;
    std::string label;
};

struct Viewport {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float min_depth = 0.0f;
    float max_depth = 1.0f;
};

struct ScissorRect {
    int32_t x = 0;
    int32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct BufferCopyRegion {
    uint64_t src_offset = 0;
    uint64_t dst_offset = 0;
    uint64_t size = 0;
};

struct TextureCopyRegion {
    uint64_t buffer_offset = 0;
    uint32_t bytes_per_row = 0; // 0 = tightly packed
    uint32_t mip_level = 0;
    uint32_t array_layer = 0;
    Offset3D origin;
    Extent3D extent;
};
