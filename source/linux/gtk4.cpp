#if defined(WINDOWKIT_BACKEND_GTK4)

#include "../../include/window.hpp"

#include <gdk/gdk.h>
#include <gtk/gtk.h>

class WindowKit::Window::Implementation
{
public:
    Implementation(int width, int height, const char* title, bool resizable, bool& fullscreen);
    ~Implementation();

    void Create();
    void Update();

    void ApplyFullscreen();

    bool Running;
    bool& Fullscreen;
    bool PreviousFullscreen;
    unsigned int Width;
    unsigned int Height;
    const char* Title;
    bool Resizable;
    bool Resized;

    GtkApplication* Application;
    GtkApplicationWindow* Window;

    static void OnActivate(GApplication* application, gpointer userData);
    static void OnClose(GtkWindow* window, gpointer userData);
};

WindowKit::Window::Implementation::Implementation(int width, int height, const char* title, bool resizable, bool& fullscreen)
    : Width(width), Height(height), Title(title), Resizable(resizable), Fullscreen(fullscreen),
      Running(true), Application(nullptr), Window(nullptr), Resized(false)
{
}

WindowKit::Window::Implementation::~Implementation()
{
    if (Application)
    {
        g_object_unref(Application);
    }
}

void WindowKit::Window::Implementation::Create()
{
    Application = gtk_application_new("com.WindowKit.Application", G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(Application, "activate", G_CALLBACK(OnActivate), this);

    g_application_register(G_APPLICATION(Application), nullptr, nullptr);
    g_application_activate(G_APPLICATION(Application));
}

void WindowKit::Window::Implementation::Update()
{
    g_main_context_iteration(nullptr, FALSE);

    if (Fullscreen != PreviousFullscreen)
    {
        ApplyFullscreen();

        PreviousFullscreen = Fullscreen;
    }

    GdkSurface* surface = gtk_native_get_surface(GTK_NATIVE(Window));

    unsigned int width = gdk_surface_get_width(surface);
    unsigned int height = gdk_surface_get_height(surface);

    Resized = width != Width or height != Height;

    if (Resized)
    {
        Width = width;
        Height = height;
    }
}

void WindowKit::Window::Implementation::ApplyFullscreen()
{
    if (Fullscreen)
    {
        gtk_window_set_decorated(GTK_WINDOW(Window), FALSE);
        gtk_window_fullscreen(GTK_WINDOW(Window));
    }
    else
    {
        gtk_window_unfullscreen(GTK_WINDOW(Window));
        gtk_window_set_decorated(GTK_WINDOW(Window), TRUE);
    }
}

void WindowKit::Window::Implementation::OnActivate(GApplication* application, gpointer userData)
{
    auto self = static_cast<Implementation*>(userData);

    self->Window = GTK_APPLICATION_WINDOW(gtk_application_window_new(GTK_APPLICATION(self->Application)));

    g_signal_connect(self->Window, "close-request", G_CALLBACK(OnClose), self);

    gtk_window_set_default_size(GTK_WINDOW(self->Window), self->Width, self->Height);
    gtk_window_set_title(GTK_WINDOW(self->Window), self->Title);
    gtk_window_set_resizable(GTK_WINDOW(self->Window), self->Resizable);
    gtk_widget_set_visible(GTK_WIDGET(self->Window), TRUE);

    self->PreviousFullscreen = not self->Fullscreen;

    self->ApplyFullscreen();
}

void WindowKit::Window::Implementation::OnClose(GtkWindow* window, gpointer userData)
{
    auto self = static_cast<Implementation*>(userData);

    self->Running = false;
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

    if (not mImplementation->Running)
    {
        mEvents.Append(Event::WindowClose);
    }

    if (mImplementation->Resized)
    {
        mEvents.Append(Event::WindowResize);

        mImplementation->Resized = false;
    }
}

#endif
