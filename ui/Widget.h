#pragma once

#include "IWidget.h"
#include "Window.h"


class Widget: public IWidget {
public:
    explicit Widget(Widget* parent = nullptr);
    [[nodiscard]] bool isTopLevel() const { return m_parent == nullptr; }
    void display() override;
private:
    Widget* m_parent = nullptr;
    Window* m_window = nullptr;
};
