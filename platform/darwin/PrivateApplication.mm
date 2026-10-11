#include "PrivateApplication.h"

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

struct PrivateApplication::Impl {
    NSApplication* m_app;
    id<NSApplicationDelegate> m_app_delegate;
};

PrivateApplication::PrivateApplication() : m_impl(std::make_unique<Impl>()) {
    m_impl->m_app = [NSApplication sharedApplication];
    m_impl->m_app_delegate = [[AppDelegate alloc] init];
    [m_impl->m_app setDelegate:m_impl->m_app_delegate];
    [m_impl->m_app setActivationPolicy:NSApplicationActivationPolicy::NSApplicationActivationPolicyRegular];
}

PrivateApplication::~PrivateApplication() = default;

void PrivateApplication::run() {
    [m_impl->m_app run];
}
