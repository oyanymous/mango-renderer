#include "Window.h"

#include <AppKit/AppKit.h>
#include <Foundation/Foundation.h>
#include <QuartzCore/CAMetalLayer.h>
#include <memory>

namespace {

struct WindowCallbacks {
    Window::PaintCallback paint;
    Window::ResizeCallback resize;
};

}

@interface WindowDelegate : NSObject <NSWindowDelegate> {
@public
    WindowCallbacks* m_callbacks;
}
@end

@implementation WindowDelegate
- (void)windowDidResize:(NSNotification*)notification {
    NSWindow* window = [notification object];
    NSView* view = [window contentView];
    NSSize size = [view convertSizeToBacking:[view bounds].size];

    if ([[view layer] isKindOfClass:[CAMetalLayer class]]) {
        CAMetalLayer* layer = (CAMetalLayer*)[view layer];
        [layer setContentsScale:[window backingScaleFactor]];
        [layer setDrawableSize:size];
    }

    if (m_callbacks->resize)
        m_callbacks->resize(static_cast<uint32_t>(size.width), static_cast<uint32_t>(size.height));
    if (m_callbacks->paint)
        m_callbacks->paint();
}
@end


struct Window::Impl {
    NSWindow* m_window;
    WindowDelegate* m_window_delegate;
    CAMetalLayer* m_metal_layer = nil;
    WindowCallbacks m_callbacks;
};

Window::Window(const std::string& title) : m_impl(std::make_unique<Impl>()), m_window_title(title) {
    NSRect content_rect = NSMakeRect(400.0, 400.0, 400.0, 400.0);
    NSWindowStyleMask style_mask = NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskTitled;
    m_impl->m_window = [[NSWindow alloc] initWithContentRect:content_rect styleMask:style_mask backing:NSBackingStoreBuffered defer:NO];
    m_impl->m_window_delegate = [[WindowDelegate alloc] init];
    m_impl->m_window_delegate->m_callbacks = &m_impl->m_callbacks;
    [m_impl->m_window setDelegate:m_impl->m_window_delegate];
}

void Window::display() {
    [m_impl->m_window makeKeyAndOrderFront:m_impl->m_window];
}

void Window::setWindowTitle(const std::string& window_title) {
    m_window_title = window_title;
    NSString *objc_window_title = [NSString stringWithUTF8String:window_title.c_str()];
    [m_impl->m_window setTitle:objc_window_title];
}

NativeSurfaceHandle Window::nativeSurface() const {
    NSView* view = [m_impl->m_window contentView];

    // Back the content view with a CAMetalLayer the first time a surface is requested.
    if (m_impl->m_metal_layer == nil) {
        m_impl->m_metal_layer = [[CAMetalLayer layer] retain];
        [m_impl->m_metal_layer setContentsScale:[m_impl->m_window backingScaleFactor]];
        [view setLayer:m_impl->m_metal_layer];
        [view setWantsLayer:YES];
    }

    NativeSurfaceHandle handle;
    handle.window = m_impl->m_window;
    handle.view = view;
    handle.layer = m_impl->m_metal_layer;
    return handle;
}

FramebufferSize Window::framebufferSize() const {
    NSView* view = [m_impl->m_window contentView];
    NSSize size = [view convertSizeToBacking:[view bounds].size];
    return {static_cast<uint32_t>(size.width), static_cast<uint32_t>(size.height)};
}

void Window::setPaintCallback(PaintCallback callback) {
    m_impl->m_callbacks.paint = std::move(callback);

    // AppKit has no WM_PAINT equivalent for a Metal-backed view; draw once the run loop starts.
    WindowCallbacks* callbacks = &m_impl->m_callbacks;
    dispatch_async(dispatch_get_main_queue(), ^{
        if (callbacks->paint)
            callbacks->paint();
    });
}

void Window::setResizeCallback(ResizeCallback callback) {
    m_impl->m_callbacks.resize = std::move(callback);
}

Window::~Window() {
    [m_impl->m_window setDelegate:nil];
    [m_impl->m_window_delegate release];
    [m_impl->m_metal_layer release];
}
