#pragma once

#include <windows.h>

#include <string>


WNDCLASSW createWindowClass(const HINSTANCE& hInstance, const WNDPROC& wndProc, const LPCWSTR& className);

HWND createWindow(const HINSTANCE& hInstance, const LPCWSTR& className, const LPCWSTR& windowTitle);

std::wstring toWide(const std::string& str);