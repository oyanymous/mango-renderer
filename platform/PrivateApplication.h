#pragma once

#include "BaseApplication.h"


class PrivateApplication : public BaseApplication
{
public:
    PrivateApplication(const PrivateApplication&) = delete;
    PrivateApplication& operator=(const PrivateApplication&) = delete;
    PrivateApplication(PrivateApplication&&) = delete;
    PrivateApplication& operator=(PrivateApplication&&) = delete;

    static PrivateApplication& instance() {
        static PrivateApplication instance;
        return instance;
    }

    void run() override;
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    PrivateApplication();
    ~PrivateApplication() override;
};