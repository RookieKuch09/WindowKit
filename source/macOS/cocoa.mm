#if defined(WINDOWKIT_BACKEND_COCOA)

#include "../../include/window.hpp"

#include <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

class WindowKit::Window::Implementation
{
public:
    Implementation(unsigned int& width, unsigned int& height, const char* title, bool resizable, bool& fullscreen);
    ~Implementation();

    void Create();
    void Update();

    void ApplyFullscreen();

    bool Running;
    bool& Fullscreen;
    bool PreviousFullscreen;
    unsigned int& Width;
    unsigned int& Height;
    const char* Title;
    bool Resizable;
    bool Resized;

    NSWindow* Window;
    id Delegate;
};

@interface WindowDelegate : NSObject <NSWindowDelegate>
@property(nonatomic, assign) WindowKit::Window::Implementation* Implementation;
@end

@implementation WindowDelegate

- (void)windowWillClose:(NSNotification*)notification
{
    if (self.Implementation)
    {
        self.Implementation->Running = false;
    }
}

- (void)windowDidResize:(NSNotification*)notification
{
    if (self.Implementation and self.Implementation->Window)
    {
        NSRect frame = [self.Implementation->Window frame];
        self.Implementation->Width = frame.size.width;
        self.Implementation->Height = frame.size.height;
        self.Implementation->Resized = true;
    }
}

@end

WindowKit::Window::Implementation::Implementation(unsigned int& width, unsigned int& height, const char* title, bool resizable, bool& fullscreen)
    : Running(true), Width(width), Height(height), Title(title), Resizable(resizable), Fullscreen(fullscreen), PreviousFullscreen(false), Window(nil), Delegate(nil)
{
}

WindowKit::Window::Implementation::~Implementation()
{
    if (Window)
    {
        [Window close];
        [Window release];
        Window = nil;
    }

    if (Delegate)
    {
        [Delegate release];
        Delegate = nil;
    }
}

void WindowKit::Window::Implementation::Create()
{
    if (not NSApp)
    {
        [NSApplication sharedApplication];
    }

    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

    if (Resizable)
    {
        style |= NSWindowStyleMaskResizable;
    }

    NSRect frame = NSMakeRect(100, 100, Width, Height);

    Window = [[NSWindow alloc] initWithContentRect:frame styleMask:style backing:NSBackingStoreBuffered defer:NO];

    [Window setTitle:[NSString stringWithUTF8String:Title]];
    [Window makeKeyAndOrderFront:nil];

    if (Resizable)
    {
        [Window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    }
    else
    {
        [[Window standardWindowButton:NSWindowZoomButton] setHidden:YES];
    }

    Delegate = [[WindowDelegate alloc] init];

    ((WindowDelegate*)Delegate).Implementation = this;

    [Window setDelegate:Delegate];

    if (Fullscreen and Resizable)
    {
        ApplyFullscreen();

        PreviousFullscreen = true;
    }
}

void WindowKit::Window::Implementation::Update()
{
    NSEvent* event = nil;

    do
    {
        event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];

        if (event)
        {
            [NSApp sendEvent:event];
            [NSApp updateWindows];
        }
    } while (event != nil);

    if (([Window styleMask] & NSWindowStyleMaskFullScreen) == NSWindowStyleMaskFullScreen)
    {
        Fullscreen = true;

        PreviousFullscreen = true;
    }
    else if ((Fullscreen != PreviousFullscreen) and Resizable)
    {
        ApplyFullscreen();

        PreviousFullscreen = Fullscreen;
    }
}

void WindowKit::Window::Implementation::ApplyFullscreen()
{
    if (not Window)
    {
        return;
    }

    dispatch_async(dispatch_get_main_queue(), ^{
      [Window toggleFullScreen:nil];
    });
}

WindowKit::Window::~Window()
{
    delete mImplementation;
}

void WindowKit::Window::Initialise()
{
    mImplementation = new Implementation(mWidth, mHeight, mTitle, mResizable, mFullscreen);
}

void WindowKit::Window::Create()
{
    mImplementation->Create();
}

void WindowKit::Window::Update()
{
    mImplementation->Update();

    if (not mImplementation->Running)
    {
        CallCallback(WindowClose{});
    }

    if (mImplementation->Resized)
    {
        CallCallback(WindowResize{.Width = mWidth, .Height = mHeight});
    }
}

#endif