#pragma once

class IWidget {
public:
    virtual ~IWidget() = default;

    virtual void display() = 0;;
};