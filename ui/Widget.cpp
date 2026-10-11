#include "Widget.h"

#include <iostream>

#include "Application.h"


Widget::Widget(Widget* parent) : m_parent(parent) {
    if (isTopLevel())
        Application::addTopLevelWidget(this);
}

void Widget::display() {
    if (m_window == nullptr) {
        m_window = new Window();
        m_window->setWindowTitle(m_window_title);
    }
    m_window->display();
}

void Widget::setWindowTitle(const std::string& title) {
    m_window_title = title;
    if (isTopLevel() and nativeWindowIsInitialized()) {
        m_window->setWindowTitle(title);
    }
}