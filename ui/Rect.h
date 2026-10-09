#pragma once

#include "Size.h"


class Rect {
public:
    Rect(int _x, int _y, int _w, int _h) : m_x(_x), m_y(_y), m_w(_w), m_h(_h) {}

    [[nodiscard]] int x() const { return m_x; }
    [[nodiscard]] int y() const { return m_y; }
    [[nodiscard]] int width() const { return m_w; }
    [[nodiscard]] int height() const { return m_h; }

    void setX(int _x) { m_x = _x; }
    void setY(int _y) { m_y = _y; }
    void setWidth(int _w) { m_w = _w; }
    void setHeight(int _h) { m_h = _h; }

    void resize(const Size& _size) {
        m_w = _size.width();
        m_h = _size.height();
    }
private:
    int m_x, m_y, m_w, m_h;
};
