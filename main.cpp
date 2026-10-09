#include "platform/Application.h"
#include "ui/Widget.h"


int main(int argc, char* argv[]) {
    Application app;

    Widget widget;
    widget.display();

    app.run();

    return 0;
}