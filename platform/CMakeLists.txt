
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PLATFORM_FILES
        darwin/PrivateApplication.mm
        darwin/Window.mm)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_FILES
        windows/PrivateApplication.cpp
        windows/PrivateApplication.h
        windows/Window.cpp
        windows/win32_helpers.cpp)
endif ()

add_library(Platform
    ${PLATFORM_FILES}
    windows/Window.h
    Application.h
    Application.cpp
    BaseApplication.h
)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(Platform PUBLIC "-framework Cocoa" Core)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(Platform PUBLIC user32 Core)
endif ()


target_include_directories(Platform
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/windows>
)
