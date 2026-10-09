#pragma once


class Point {
public:
    Point() : m_x(100), m_y(100) {}
    Point(int _x, int _y) : m_x(_x), m_y(_y) {}

    [[nodiscard]] int x() const { return m_x; }
    [[nodiscard]] int y() const { return m_y; }

    void setX(int _x) { m_x = _x; }
    void setY(int _y) { m_y = _y; }

    Point operator+(const Point& other) const {
        return Point(m_x + other.m_x, m_y + other.m_y);
    }

    Point operator-(const Point& other) const {
        return Point(m_x - other.m_x, m_y - other.m_y);
    }
private:
    int m_x, m_y;
};

