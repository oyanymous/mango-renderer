
if (APPLE)
    enable_language(OBJCXX)
endif ()

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(ui)
add_subdirectory(gpu)
add_subdirectory(renderer)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE Platform UI Renderer)
