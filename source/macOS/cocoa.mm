#if defined(WINDOWKIT_BACKEND_COCOA)

#include "../../include/window.hpp"

#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>

@interface WindowDelegate : NSObject <NSWindowDelegate>
@property(nonatomic, assign) WindowKit::Window::Implementation* Implementation;
@end

@interface ApplicationDelegate : NSObject <NSApplicationDelegate>
@property(nonatomic, assign) NSWindow* window;
@end

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
    WindowDelegate* Delegate;
    ApplicationDelegate* AppDelegate;
};

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

- (void)windowDidMove:(NSNotification*)notification
{
    NSWindow* window = notification.object;
    NSString* key = @"WindowFrame";
    NSString* frameString = NSStringFromRect([window frame]);
    [[NSUserDefaults standardUserDefaults] setObject:frameString forKey:key];
}

@end

@implementation ApplicationDelegate

- (BOOL)applicationShouldHandleReopen:(NSApplication*)sender hasVisibleWindows:(BOOL)flag
{
    if (self.window and not[self.window isVisible])
    {
        [self.window makeKeyAndOrderFront:nil];
    }
    return YES;
}

@end

WindowKit::Window::Implementation::Implementation(unsigned int& width, unsigned int& height, const char* title, bool resizable, bool& fullscreen)
    : Running(true),
      Width(width),
      Height(height),
      Title(title),
      Resizable(resizable),
      Fullscreen(fullscreen),
      PreviousFullscreen(false),
      Window(nil),
      Delegate(nil),
      AppDelegate(nil)
{
}

WindowKit::Window::Implementation::~Implementation()
{
    [[NSNotificationCenter defaultCenter] removeObserver:NSApp name:NSApplicationDidBecomeActiveNotification object:nil];

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

    if (AppDelegate)
    {
        [NSApp setDelegate:nil];
        [AppDelegate release];
        AppDelegate = nil;
    }
}

void WindowKit::Window::Implementation::Create()
{
    if (!NSApp)
    {
        [NSApplication sharedApplication];
    }

    NSString* appName = [[NSProcessInfo processInfo] processName];

    NSMenu* mainMenu = [[NSMenu alloc] initWithTitle:@"MainMenu"];
    [NSApp setMainMenu:mainMenu];

    NSMenuItem* appMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    [mainMenu addItem:appMenuItem];

    NSMenu* appMenu = [[NSMenu alloc] initWithTitle:appName];

    NSString* quitTitle = [NSString stringWithFormat:@"Quit %@", appName];
    NSMenuItem* quitItem = [[NSMenuItem alloc] initWithTitle:quitTitle
                                                      action:@selector(terminate:)
                                               keyEquivalent:@"q"];
    [appMenu addItem:quitItem];

    [appMenuItem setSubmenu:appMenu];

    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
    if (Resizable)
    {
        style |= NSWindowStyleMaskResizable;
    }

    NSString* frameString = [[NSUserDefaults standardUserDefaults] objectForKey:@"WindowFrame"];
    NSRect frame;
    if (frameString != nil)
    {
        frame = NSRectFromString(frameString);
    }
    else
    {
        frame = NSMakeRect(0, 0, Width, Height);
    }

    Window = [[NSWindow alloc] initWithContentRect:frame
                                         styleMask:style
                                           backing:NSBackingStoreBuffered
                                             defer:YES];

    [Window setTitle:[NSString stringWithUTF8String:Title]];

    Delegate = [[WindowDelegate alloc] init];
    Delegate.Implementation = this;
    [Window setDelegate:Delegate];

    if (Resizable)
    {
        [Window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    }
    else
    {
        [[Window standardWindowButton:NSWindowZoomButton] setHidden:YES];
    }

    AppDelegate = [[ApplicationDelegate alloc] init];
    AppDelegate.window = Window;
    [NSApp setDelegate:AppDelegate];

    [NSApp finishLaunching];
    [Window makeKeyAndOrderFront:nil];

    dispatch_async(dispatch_get_main_queue(), ^{
      [NSApp activateIgnoringOtherApps:YES];
    });

    if (Fullscreen && Resizable)
    {
        ApplyFullscreen();
        PreviousFullscreen = true;
    }

    [[NSNotificationCenter defaultCenter] addObserverForName:NSApplicationDidBecomeActiveNotification
                                                      object:nil
                                                       queue:[NSOperationQueue mainQueue]
                                                  usingBlock:^(NSNotification* _Nonnull note) {
                                                    if (Window && [Window isMiniaturized])
                                                    {
                                                        [Window deminiaturize:nil];
                                                        [Window makeKeyAndOrderFront:nil];
                                                    }
                                                  }];

    if (frameString == nil)
    {
        [Window center];
    }
}

void WindowKit::Window::Implementation::Update()
{
    NSEvent* event = nil;

    do
    {
        event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                   untilDate:[NSDate distantPast]
                                      inMode:NSDefaultRunLoopMode
                                     dequeue:YES];

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
    if (!Window)
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

    if (!mImplementation->Running)
    {
        CallCallback(WindowClose{});
    }

    if (mImplementation->Resized)
    {
        CallCallback(WindowResize{.Width = mWidth, .Height = mHeight});
        mImplementation->Resized = false;
    }
}

#endif