#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "FrameContext.h"
#include "IGraphicsDevice.h"
#include "Window.h"


class Renderer {
public:
    static constexpr uint32_t MaxFramesInFlight = 2;

    explicit Renderer(Window& window, GraphicsAPI api = GraphicsAPI::Auto);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Waits for the frame slot and acquires a back buffer; returns false if the frame must be skipped.
    bool beginFrame();
    void render();
    void endFrame();
    void resize(uint32_t width, uint32_t height);

    IGraphicsDevice& device() { return *m_device; }
private:
    void createTrianglePipeline();

    Window& m_window;
    std::unique_ptr<IGraphicsDevice> m_device;
    std::unique_ptr<ISwapchain> m_swapchain;
    std::array<FrameContext, MaxFramesInFlight> m_frames;
    uint32_t m_frame_index = 0;
    ITexture* m_back_buffer = nullptr;

    std::unique_ptr<IPipelineLayout> m_pipeline_layout;
    std::unique_ptr<IGraphicsPipeline> m_pipeline;
    std::unique_ptr<IBuffer> m_vertex_buffer;
};
