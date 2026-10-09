cmake_minimum_required(VERSION 3.24)
project(MangoRenderer LANGUAGES CXX OBJCXX)

add_subdirectory(platform)

add_executable(main main.cpp)


target_link_libraries(main PRIVATE Platform)
