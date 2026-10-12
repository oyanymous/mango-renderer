#pragma once

#include <memory>

#include "IGraphicsDevice.h"


class GraphicsDeviceFactory {
public:
    GraphicsDeviceFactory() = delete;

    // Throws std::runtime_error if the requested API is not compiled in or cannot be initialized.
    static std::unique_ptr<IGraphicsDevice> create(const DeviceDesc& desc);
    static bool isSupported(GraphicsAPI api);
    static GraphicsAPI defaultAPI();
};
