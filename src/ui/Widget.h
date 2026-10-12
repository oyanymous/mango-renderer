#pragma once

#include "IWidget.h"
#include "../platform/Window.h"


class Widget: public IWidget {
public:
    explicit Widget(Widget* parent = nullptr);
    [[nodiscard]] bool isTopLevel() const { return m_parent == nullptr; }
    void display() override;
    void setWindowTitle(const std::string& title);
    [[nodiscard]] const std::string& getWindowTitle() const { return m_window_title; }
    // The native window, or nullptr until display() has been called.
    [[nodiscard]] Window* window() const { return m_window; }
private:
    Widget* m_parent = nullptr;
    Window* m_window = nullptr;
    std::string m_window_title;

    [[nodiscard]] bool nativeWindowIsInitialized() const { return m_window != nullptr; }
};
