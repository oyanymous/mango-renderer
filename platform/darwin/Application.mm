#include "Application.h"

#include <AppKit/AppKit.h>
#include <memory>


@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@interface AppDelegate ()

@property (strong) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}

@end

struct Application::Impl {
    NSApplication* m_app;
    id<NSApplicationDelegate> m_app_delegate;
};

Application::Application() : m_impl(std::make_unique<Impl>()) {
    m_impl->m_app = [NSApplication sharedApplication];
    m_impl->m_app_delegate = [[AppDelegate alloc] init];
    [m_impl->m_app setDelegate:m_impl->m_app_delegate];
    [m_impl->m_app setActivationPolicy:NSApplicationActivationPolicy::NSApplicationActivationPolicyRegular];
}

void Application::run() {
    [m_impl->m_app run];
}
Application::~Application() = default;
