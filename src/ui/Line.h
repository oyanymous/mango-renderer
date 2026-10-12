#pragma once

#include "Point.h"


class Line {
public:
    Line(int x0, int y0, int x1, int y1) : p0(x0, y0), p1(x1, y1) {}
private:
    Point p0, p1;
};
