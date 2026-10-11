#include "PrivateApplication.h"

#include <windows.h>

PrivateApplication::PrivateApplication() = default;

PrivateApplication::~PrivateApplication() = default;

void PrivateApplication::run() {
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}