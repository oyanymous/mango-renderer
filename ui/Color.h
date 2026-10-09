#pragma once


class Color {
public:
    Color() : m_r(0), m_g(0), m_b(0), m_a(255) {}

    Color(int _r, int _g, int _b, int _a = 255) : m_r(_r), m_g(_g), m_b(_b), m_a(_a) {}

    [[nodiscard]] int red() const { return m_r; }
    [[nodiscard]] int green() const { return m_g; }
    [[nodiscard]] int blue() const { return m_b; }
    [[nodiscard]] int alpha() const { return m_a; }

    [[nodiscard]] float redF() const { return m_r / 255.0f; }
    [[nodiscard]] float greenF() const { return m_g / 255.0f; }
    [[nodiscard]] float blueF() const { return m_b / 255.0f; }
    [[nodiscard]] float alphaF() const { return m_a / 255.0f; }
private:
    int m_r, m_g, m_b, m_a;
};

