#pragma once

#include <memory>

class Application {
public:
    Application();
    ~Application();
    void run();
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};