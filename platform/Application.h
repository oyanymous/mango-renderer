#pragma once

#include "PrivateApplication.h"

#include "IWidget.h"


class Application {
public:
    Application();

    static void addTopLevelWidget(IWidget* widget) {
        PrivateApplication::instance().addTopLevelWidget(widget);
    }

    void run();
};