#pragma once

#include <memory>

class Window {
public:
    Window();
    ~Window();
    void display();
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};