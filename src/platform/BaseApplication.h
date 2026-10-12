#pragma once

#include <vector>

#include "IWidget.h"


class BaseApplication {
public:
    virtual ~BaseApplication() = default;

    void addTopLevelWidget(IWidget* widget) {
        m_top_level_widgets.push_back(widget);
    }

    [[nodiscard]] std::vector<IWidget*> topLevelWidgets() { return m_top_level_widgets; }

    virtual void run() = 0;
private:
    std::vector<IWidget*> m_top_level_widgets;
};