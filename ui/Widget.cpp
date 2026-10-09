#include "Widget.h"
#include "Application.h"


Widget::Widget(Widget* parent) : m_parent(parent) {
    if (isTopLevel())
        Application::addTopLevelWidget(this);
}

void Widget::display() {
    if (m_window == nullptr) {
        m_window = new Window();
    }
    m_window->display();
}
