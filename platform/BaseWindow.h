#pragma once

#include <string>


class BaseWindow {
public:
    virtual ~BaseWindow() = default;
    virtual void display() = 0;
    virtual void setWindowTitle(const std::string& title) = 0;
};