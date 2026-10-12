#include <exception>
#include <iostream>

#include "platform/Application.h"
#include "renderer/Renderer.h"
#include "ui/Widget.h"


int main(int argc, char* argv[]) {
    Application app;

    Widget widget;
    widget.setWindowTitle("Parham is the best software developer in the world!");
    widget.display();

    Window& window = *widget.window();

    try {
        Renderer renderer(window, GraphicsAPI::Vulkan);
        std::cout << "Rendering with " << renderer.device().capabilities().device_name << '\n';

        bool rendering = true;
        window.setResizeCallback([&](uint32_t width, uint32_t height) {
            if (rendering)
                renderer.resize(width, height);
        });
        window.setPaintCallback([&] {
            if (!rendering)
                return;
            // Exceptions must not unwind through the OS window procedure.
            try {
                if (renderer.beginFrame()) {
                    renderer.render();
                    renderer.endFrame();
                }
            } catch (const std::exception& e) {
                std::cerr << "Rendering failed: " << e.what() << '\n';
                rendering = false;
            }
        });

        app.run();

        window.setPaintCallback(nullptr);
        window.setResizeCallback(nullptr);
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize the renderer: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
