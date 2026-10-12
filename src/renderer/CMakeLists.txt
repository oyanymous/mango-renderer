include(VulkanDeps)

add_library(Renderer
    Renderer.h
    Renderer.cpp
    FrameContext.h
    shaders/triangle.vert
    shaders/triangle.frag
)

# Shaders are authored in GLSL and compiled to SPIR-V headers at build time.
mango_compile_shaders(Renderer
    shaders/triangle.vert
    shaders/triangle.frag
)

target_link_libraries(Renderer PUBLIC Gpu Platform)

target_include_directories(Renderer
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
