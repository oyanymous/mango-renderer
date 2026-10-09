#pragma once

#include <memory>
#include <vector>

#include "IWidget.h"


class PrivateApplication {
public:
    PrivateApplication(const PrivateApplication&) = delete;
    PrivateApplication& operator=(const PrivateApplication&) = delete;
    PrivateApplication(PrivateApplication&&) = delete;
    PrivateApplication& operator=(PrivateApplication&&) = delete;

    static PrivateApplication& instance() {
        static PrivateApplication instance;
        return instance;
    }

    void addTopLevelWidget(IWidget* widget) {
        m_top_level_widgets.push_back(widget);
    }

    [[nodiscard]] std::vector<IWidget*> topLevelWidgets() { return m_top_level_widgets; }

    void run();
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    std::vector<IWidget*> m_top_level_widgets;

    PrivateApplication();
    ~PrivateApplication();
};