#pragma once

#include "Point.h"
#include <vector>


class Polygon {
public:
    using Points = std::vector<Point>;

    Polygon() = default;

    [[nodiscard]] const Points& points() const { return m_points; }
private:
    std::vector<Point> m_points;
};
