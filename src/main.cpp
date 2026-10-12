#include "platform/Application.h"
#include "ui/Widget.h"


int main(int argc, char* argv[]) {
    Application app;

    Widget widget;
    widget.setWindowTitle("Parham is the best software developer in the world!");
    widget.display();

    app.run();

    return 0;
}