
add_library(Platform Application.h darwin/Application.mm Window.h darwin/Window.mm)

target_include_directories(Platform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Platform PUBLIC "-framework Cocoa")

