#include "Application.h"
#include "PrivateApplication.h"

Application::Application() {
    // Force NSApplication setup before any window is created.
    PrivateApplication::instance();
}

void Application::run() {
    PrivateApplication::instance().run();
    for (IWidget* widget : PrivateApplication::instance().topLevelWidgets()) {
        
    }
}
