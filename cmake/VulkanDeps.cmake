include_guard(GLOBAL)

include(FetchContent)

set(MANGO_VULKAN_SDK_TAG "vulkan-sdk-1.4.357.0")

FetchContent_Declare(VulkanHeaders
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG ${MANGO_VULKAN_SDK_TAG}
    GIT_SHALLOW TRUE
)

# glslang is only used for its standalone compiler, which turns GLSL into
# SPIR-V headers at build time (see mango_compile_shaders below).
set(ENABLE_OPT OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES ON CACHE BOOL "" FORCE)
set(GLSLANG_TESTS OFF CACHE BOOL "" FORCE)
set(GLSLANG_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_EXTERNAL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG ${MANGO_VULKAN_SDK_TAG}
    GIT_SHALLOW TRUE
)

# Header-only; SOURCE_SUBDIR points at a directory that does not exist so
# FetchContent only downloads it and does not run VMA's own CMakeLists.
FetchContent_Declare(VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.3.0
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR do-not-build
)

FetchContent_MakeAvailable(VulkanHeaders glslang VulkanMemoryAllocator)

# glslang links fully static on MinGW, which fails against some MinGW-w64 builds
# (undefined __intrinsic_setjmpex from libwinpthread.a). Keep only the GCC runtimes static.
if (MINGW)
    set_property(TARGET glslang-standalone PROPERTY LINK_OPTIONS -static-libgcc -static-libstdc++)
endif ()

if (NOT TARGET VulkanMemoryAllocator::Headers)
    add_library(VulkanMemoryAllocatorHeaders INTERFACE)
    add_library(VulkanMemoryAllocator::Headers ALIAS VulkanMemoryAllocatorHeaders)
    target_include_directories(VulkanMemoryAllocatorHeaders SYSTEM INTERFACE
        "${vulkanmemoryallocator_SOURCE_DIR}/include")
    target_link_libraries(VulkanMemoryAllocatorHeaders INTERFACE Vulkan::Headers)
endif ()

# mango_compile_shaders(<target> <shader>...)
#
# Compiles each GLSL shader to SPIR-V with glslang and emits a header
# `<name>_<stage>.spv.h` declaring `const uint32_t <name>_<stage>_spv[]`.
# The generated directory is added to <target>'s include path.
function(mango_compile_shaders target)
    set(out_dir "${CMAKE_CURRENT_BINARY_DIR}/shaders")
    file(MAKE_DIRECTORY "${out_dir}")

    set(outputs)
    foreach (shader IN LISTS ARGN)
        get_filename_component(shader_path "${shader}" ABSOLUTE)
        get_filename_component(shader_name "${shader}" NAME_WE)
        get_filename_component(shader_ext "${shader}" LAST_EXT)
        string(SUBSTRING "${shader_ext}" 1 -1 shader_stage)

        set(variable "${shader_name}_${shader_stage}_spv")
        set(output "${out_dir}/${shader_name}_${shader_stage}.spv.h")

        add_custom_command(
            OUTPUT "${output}"
            COMMAND glslang-standalone -V --target-env vulkan1.3
                    --vn "${variable}" -o "${output}" "${shader_path}"
            DEPENDS "${shader_path}" glslang-standalone
            COMMENT "Compiling shader ${shader_name}.${shader_stage}"
            VERBATIM
        )
        list(APPEND outputs "${output}")
    endforeach ()

    target_sources(${target} PRIVATE ${outputs})
    target_include_directories(${target} PRIVATE "${out_dir}")
endfunction()
