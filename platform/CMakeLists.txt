
add_library(Platform 
    PrivateApplication.h
    darwin/PrivateApplication.mm 
    Window.h 
    darwin/Window.mm
    Application.h
    Application.cpp
)

target_link_libraries(Platform PUBLIC "-framework Cocoa" Core)

target_include_directories(Platform
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
