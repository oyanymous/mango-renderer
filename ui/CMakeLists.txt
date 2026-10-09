
add_library(UI Widget.h Widget.cpp
    Size.h
    Rect.h
    Rect.cpp
    Polygon.h
    Polygon.cpp
    Point.cpp
    Line.h
    Line.cpp
    Color.h
    Color.cpp
    Point.h)

target_link_libraries(UI PRIVATE Platform Core)

target_include_directories(UI PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)