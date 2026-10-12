
if (APPLE)
    set(PLATFORM_FILES
        darwin/PrivateApplication.mm
        darwin/Window.mm)
elseif (WIN32)
    set(PLATFORM_FILES
        windows/PrivateApplication.cpp
        windows/Window.cpp
        windows/win32_helpers.h
        windows/win32_helpers.cpp)
endif ()

add_library(Platform
    ${PLATFORM_FILES}
    Application.h
    Application.cpp
    BaseApplication.h
    Window.h
    NativeSurfaceHandle.h
    PrivateApplication.h
)

if (APPLE)
    target_link_libraries(Platform PUBLIC "-framework Cocoa" "-framework QuartzCore" Core)
elseif (WIN32)
    target_link_libraries(Platform PUBLIC user32 Core)
endif ()

target_include_directories(Platform
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
