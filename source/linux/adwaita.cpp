#if defined(WINDOWKIT_BACKEND_ADWAITA)

#include "../../include/window.hpp"

#include <adwaita.h>
#include <gtk/gtk.h>

class WindowKit::Window::Implementation
{
public:
    Implementation(int width, int height, const char* title, bool resizable, bool& fullscreen);
    ~Implementation();

    void Create();
    void Update();
    bool Active();

    // GTK objects
    AdwApplication* mApplication;
    GtkApplicationWindow* mWindow;
    GtkWidget* mHeaderBar;

    // Signal handler ID for the fullscreen notifier
    gulong mFullscreenHandlerID = 0;

    // Window state
    bool IsOpen;
    bool& Fullscreen;
    unsigned int Width;
    unsigned int Height;
    const char* Title;
    bool Resizable;

    static bool mInitialised;

    // Signal callbacks
    static gboolean OnClose(GtkWindow* window, gpointer userData);
    static void OnFullscreen(GObject* obj, GParamSpec* pspec, gpointer userData);
    static void OnDestroy(GtkWidget* widget, gpointer userData);
};

bool WindowKit::Window::Implementation::mInitialised = false;

WindowKit::Window::Implementation::Implementation(int width, int height, const char* title, bool resizable, bool& fullscreen)
    : Width(width), Height(height), Title(title), Resizable(resizable), Fullscreen(fullscreen), IsOpen(true), mApplication(nullptr), mWindow(nullptr), mHeaderBar(nullptr)
{
}

WindowKit::Window::Implementation::~Implementation()
{
    // If the window is still alive, unref it now
    if (mWindow)
    {
        g_object_unref(mWindow);
        mWindow = nullptr;
    }
    if (mApplication)
    {
        g_object_unref(mApplication);
        mApplication = nullptr;
    }
}

void WindowKit::Window::Implementation::Create()
{
    // Create the Adwaita application
    mApplication = adw_application_new("com.WindowKit.App", G_APPLICATION_DEFAULT_FLAGS);

    if (!mInitialised)
    {
        adw_init();
        mInitialised = true;
    }

    g_application_register(G_APPLICATION(mApplication), nullptr, nullptr);

    // Instantiate a new window
    mWindow = GTK_APPLICATION_WINDOW(
        g_object_new(ADW_TYPE_APPLICATION_WINDOW,
                     "application", mApplication,
                     "default-width", Width,
                     "default-height", Height,
                     nullptr));

    // Connect the close-request (user clicks the X)
    g_signal_connect(mWindow,
                     "close-request",
                     G_CALLBACK(Implementation::OnClose),
                     this);

    // Build a header bar
    mHeaderBar = adw_header_bar_new();
    adw_header_bar_set_title_widget(ADW_HEADER_BAR(mHeaderBar),
                                    gtk_label_new(Title));

    // Add header to a toolbar view
    GtkWidget* tv = adw_toolbar_view_new();
    adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(tv), mHeaderBar);

    adw_application_window_set_content(ADW_APPLICATION_WINDOW(mWindow), tv);

    // Show the window
    gtk_window_present(GTK_WINDOW(mWindow));

    // Honor requested fullscreen at launch
    if (Fullscreen)
    {
        gtk_window_fullscreen(GTK_WINDOW(mWindow));
    }

    // 1) Track runtime fullscreen changes
    mFullscreenHandlerID = g_signal_connect(
        mWindow,
        "notify::fullscreened",
        G_CALLBACK(Implementation::OnFullscreen),
        this);

    // 2) Clean up on destroy: disconnect signals & unref
    g_signal_connect(
        mWindow,
        "destroy",
        G_CALLBACK(Implementation::OnDestroy),
        this);
}

void WindowKit::Window::Implementation::Update()
{
    while (g_main_context_pending(nullptr))
    {
        g_main_context_iteration(nullptr, FALSE);
    }
}

bool WindowKit::Window::Implementation::Active()
{
    return IsOpen;
}

gboolean WindowKit::Window::Implementation::OnClose(GtkWindow* window, gpointer userData)
{
    // Simply mark the window as closed and stop default destruction
    auto self = static_cast<Implementation*>(userData);
    self->IsOpen = false;
    return GDK_EVENT_STOP;
}

void WindowKit::Window::Implementation::OnFullscreen(GObject* obj,
                                                     GParamSpec* /*pspec*/,
                                                     gpointer userData)
{
    // Called whenever the 'fullscreened' property changes
    auto self = static_cast<Implementation*>(userData);
    bool isFs = gtk_window_is_fullscreen(GTK_WINDOW(obj));
    self->Fullscreen = isFs;

    // Hide header bar in fullscreen, show otherwise
    gtk_widget_set_visible(self->mHeaderBar, !isFs);
}

void WindowKit::Window::Implementation::OnDestroy(GtkWidget* widget,
                                                  gpointer userData)
{
    auto self = static_cast<Implementation*>(userData);

    // 1) Disconnect fullscreen notifier
    if (self->mFullscreenHandlerID != 0)
    {
        g_signal_handler_disconnect(widget, self->mFullscreenHandlerID);
        self->mFullscreenHandlerID = 0;
    }

    // 2) Unref the window here, since GTK is done with it
    if (self->mWindow)
    {
        g_object_unref(self->mWindow);
        self->mWindow = nullptr;
    }
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

    mEvents.Purge();

    if (not mImplementation->Active())
    {
        mEvents.Append(Event::WindowClose);
    }
}

#endif