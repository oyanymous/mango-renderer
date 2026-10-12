#include "Renderer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include "GraphicsDeviceFactory.h"

// Generated at build time from shaders/*.{vert,frag} by mango_compile_shaders().
#include "triangle_vert.spv.h"
#include "triangle_frag.spv.h"


namespace {

struct Vertex {
    float position[2];
    float color[3];
};

// Clip space, Vulkan convention (+y points down).
constexpr Vertex kTriangle[] = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
};

template <size_t N>
ShaderSource spirv(const uint32_t (&words)[N]) {
    ShaderSource source;
    source.language = ShaderLanguage::SPIRV;
    source.code.resize(sizeof(words));
    std::memcpy(source.code.data(), words, sizeof(words));
    return source;
}

}


Renderer::Renderer(Window& window, GraphicsAPI api) : m_window(window) {
    DeviceDesc device_desc;
    device_desc.api = api;
    device_desc.surface = window.nativeSurface();
#ifndef NDEBUG
    device_desc.enable_validation = true;
#endif
    m_device = GraphicsDeviceFactory::create(device_desc);

    const FramebufferSize size = window.framebufferSize();
    SwapchainDesc swapchain_desc;
    swapchain_desc.surface = device_desc.surface;
    swapchain_desc.extent = {size.width, size.height};
    swapchain_desc.format = TextureFormat::BGRA8Unorm;
    m_swapchain = m_device->createSwapchain(swapchain_desc);

    for (FrameContext& frame : m_frames) {
        frame.command_buffer = m_device->graphicsQueue().createCommandBuffer();
        frame.in_flight_fence = m_device->createFence(true);
    }

    createTrianglePipeline();
}

Renderer::~Renderer() {
    m_device->waitIdle();
}

bool Renderer::beginFrame() {
    FrameContext& frame = m_frames[m_frame_index];
    frame.in_flight_fence->wait(UINT64_MAX);

    m_back_buffer = m_swapchain->acquireNextImage();
    if (!m_back_buffer)
        return false;

    // Only reset once we know this frame will be submitted, otherwise the next wait would deadlock.
    frame.in_flight_fence->reset();
    return true;
}

void Renderer::render() {
    ICommandBuffer& cmd = *m_frames[m_frame_index].command_buffer;
    const Extent2D extent = m_swapchain->extent();

    cmd.begin();

    RenderPassDesc pass;
    pass.label = "Triangle";
    ColorAttachment& color = pass.color_attachments.emplace_back();
    color.texture = m_back_buffer;
    color.load_op = LoadOp::Clear;
    color.store_op = StoreOp::Store;
    color.clear_color = {0.08f, 0.08f, 0.10f, 1.0f};

    cmd.beginRenderPass(pass);
    cmd.setViewport({0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f});
    cmd.setScissor({0, 0, extent.width, extent.height});
    cmd.setGraphicsPipeline(*m_pipeline);
    cmd.setVertexBuffer(0, *m_vertex_buffer, 0);
    cmd.draw(3, 1, 0, 0);
    cmd.endRenderPass();

    cmd.end();
}

void Renderer::endFrame() {
    FrameContext& frame = m_frames[m_frame_index];
    ICommandQueue& queue = m_device->graphicsQueue();

    ICommandBuffer* buffers[] = {frame.command_buffer.get()};
    queue.submit(buffers, frame.in_flight_fence.get());
    queue.present(*m_swapchain);

    m_back_buffer = nullptr;
    m_frame_index = (m_frame_index + 1) % MaxFramesInFlight;
}

void Renderer::resize(uint32_t width, uint32_t height) {
    m_swapchain->resize(width, height);
}

void Renderer::createTrianglePipeline() {
    ShaderModuleDesc vertex_desc;
    vertex_desc.stage = ShaderStage::Vertex;
    vertex_desc.sources.push_back(spirv(triangle_vert_spv));
    auto vertex_shader = m_device->createShaderModule(vertex_desc);

    ShaderModuleDesc fragment_desc;
    fragment_desc.stage = ShaderStage::Fragment;
    fragment_desc.sources.push_back(spirv(triangle_frag_spv));
    auto fragment_shader = m_device->createShaderModule(fragment_desc);

    m_pipeline_layout = m_device->createPipelineLayout({});

    GraphicsPipelineDesc pipeline_desc;
    pipeline_desc.vertex_shader = vertex_shader.get();
    pipeline_desc.fragment_shader = fragment_shader.get();
    pipeline_desc.layout = m_pipeline_layout.get();
    pipeline_desc.vertex_layout.buffers = {{0, sizeof(Vertex), VertexStepMode::Vertex}};
    pipeline_desc.vertex_layout.attributes = {
        {0, 0, VertexFormat::Float2, offsetof(Vertex, position)},
        {1, 0, VertexFormat::Float3, offsetof(Vertex, color)},
    };
    pipeline_desc.topology = PrimitiveTopology::TriangleList;
    pipeline_desc.color_formats = {m_swapchain->format()};
    m_pipeline = m_device->createGraphicsPipeline(pipeline_desc);
    m_pipeline->setDebugName("Triangle pipeline");

    m_vertex_buffer = m_device->createBuffer({sizeof(kTriangle), BufferUsage::Vertex, MemoryType::GpuOnly}, kTriangle);
    m_vertex_buffer->setDebugName("Triangle vertices");
}
