
add_library(Core INTERFACE IWidget.h)

target_include_directories(Core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)