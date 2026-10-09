cmake_minimum_required(VERSION 3.24)
project(MangoRenderer LANGUAGES CXX OBJCXX)

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(ui)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE Platform UI)
