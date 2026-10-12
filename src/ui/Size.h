#pragma once


class Size {
public:
    Size(int w, int h) : m_w(w), m_h(h) {}
    
    [[nodiscard]] int width() const { return m_w; }
    [[nodiscard]] int height() const { return m_h; }
    
    void setWidth(int w) { m_w = w; }
    void setHeight(int h) { m_h = h; }
private:
    int m_w, m_h;
};

