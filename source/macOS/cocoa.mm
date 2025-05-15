#if defined(WINDOWKIT_BACKEND_COCOA)

#include "../../include/window.hpp"

#include <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

class WindowKit::Window::Implementation
{
public:
    Implementation(int width, int height, const char* title, bool resizable, bool& fullscreen);
    ~Implementation();

    void Create();
    void Update();

    bool Active();

    NSWindow* Window;
    id Delegate;
    bool IsOpen;

    unsigned int Width;
    unsigned int Height;
    const char* Title;

    bool Resizable;
    bool& Fullscreen;
    bool LastFrameFullscreen;
};

@interface WindowDelegate : NSObject <NSWindowDelegate>
@property(nonatomic, assign) WindowKit::Window::Implementation* Implementation;
@end

@implementation WindowDelegate
- (void)windowWillClose:(NSNotification*)notification
{
    if (self.Implementation)
    {
        ((WindowKit::Window::Implementation*)self.Implementation)->IsOpen = false;
    }
}

@end

WindowKit::Window::Implementation::Implementation(int width, int height, const char* title, bool resizable, bool& fullscreen)
    : Window(nullptr), Delegate(nil), IsOpen(true), Width(width), Height(height), Title(title), Resizable(resizable), Fullscreen(fullscreen), LastFrameFullscreen(false)
{
}

WindowKit::Window::Implementation::~Implementation()
{
    [Window close];
    Delegate = nil;
}

void WindowKit::Window::Implementation::Create()
{
    NSRect frame = NSMakeRect(100, 100, Width, Height);
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

    if (Resizable)
    {
        style |= NSWindowStyleMaskResizable;
    }

    Window = [[NSWindow alloc] initWithContentRect:frame styleMask:style backing:NSBackingStoreBuffered defer:NO];

    [Window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

    [Window setTitle:[NSString stringWithUTF8String:Title]];
    [Window makeKeyAndOrderFront:nil];

    Delegate = [[WindowDelegate alloc] init];
    ((WindowDelegate*)Delegate).Implementation = this;
    [Window setDelegate:Delegate];
}

void WindowKit::Window::Implementation::Update()
{
    NSEvent* event;
    do
    {
        event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];

        if (event)
        {
            [NSApp sendEvent:event];
            [NSApp updateWindows];
        }
    } while (event);

    if (Fullscreen != LastFrameFullscreen)
    {
        dispatch_async(dispatch_get_main_queue(), ^{
          [Window toggleFullScreen:nil];
        });
        LastFrameFullscreen = Fullscreen;
    }
}

bool WindowKit::Window::Implementation::Active()
{
    return IsOpen;
}

WindowKit::Window::~Window()
{
    delete mImplementation;
}

void WindowKit::Window::Initialise()
{
    mImplementation = new Implementation(mWidth, mHeight, mTitle, mResizable, mFullscreen);

    [NSApplication sharedApplication];
}

void WindowKit::Window::Create()
{
    mImplementation->Create();
}

void WindowKit::Window::Update()
{
    mImplementation->Update();

    mEvents.Purge();

    if (not mImplementation->Active())
    {
        mEvents.Append(Event::WindowClose);
    }
}

#endif