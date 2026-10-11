#include "../PrivateApplication.h"

#include <windows.h>

struct PrivateApplication::Impl {
};

PrivateApplication::PrivateApplication() : m_impl(std::make_unique<Impl>()){

}

PrivateApplication::~PrivateApplication() = default;

void PrivateApplication::run() {
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}