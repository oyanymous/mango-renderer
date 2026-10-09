#include "Window.h"
#include <AppKit/AppKit.h>
#include <Foundation/Foundation.h>
#include <memory>

@interface WindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation WindowDelegate
@end


struct Window::Impl {
    NSWindow* m_window;
    WindowDelegate* m_window_delegate;
};

Window::Window() : m_impl(std::make_unique<Impl>()) {
    NSRect content_rect = NSMakeRect(400.0, 400.0, 400.0, 400.0);
    NSWindowStyleMask style_mask = NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskTitled;
    m_impl->m_window = [[NSWindow alloc] initWithContentRect:content_rect styleMask:style_mask backing:NSBackingStoreBuffered defer:NO];
    m_impl->m_window_delegate = [[WindowDelegate alloc] init];
    [m_impl->m_window setDelegate:m_impl->m_window_delegate];
}

void Window::display() {
    [m_impl->m_window makeKeyAndOrderFront:m_impl->m_window];
}

Window::~Window() = default;
