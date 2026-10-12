#pragma once

#include <memory>

#include "BaseWindow.h"

class Window: public BaseWindow {
public:
    explicit Window(const std::string& window_title = "");
    ~Window() override;

    void display() override;
    void setWindowTitle(const std::string& title) override;
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    std::string m_window_title;
};