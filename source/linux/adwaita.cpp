#if defined(WINDOWKIT_BACKEND_ADWAITA)

#include "../../include/window.hpp"

#include <adwaita.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

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

    AdwApplication* Application;
    AdwApplicationWindow* Window;
    AdwHeaderBar* Header;
    AdwToolbarView* Toolbar;

    GtkWidget* Content;

    static void OnActivate(GApplication* application, gpointer userData);
    static void OnClose(GtkWindow* window, gpointer userData);
};

WindowKit::Window::Implementation::Implementation(unsigned int& width, unsigned int& height, const char* title, bool resizable, bool& fullscreen)
    : Width(width), Height(height), Title(title), Resizable(resizable), Fullscreen(fullscreen), Running(true), Application(nullptr), Window(nullptr), Header(nullptr), Toolbar(nullptr), Content(nullptr), Resized(false)
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
    Application = ADW_APPLICATION(adw_application_new("com.WindowKit.Application", G_APPLICATION_DEFAULT_FLAGS));

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

    unsigned int width = gtk_widget_get_width(Content);
    unsigned int height = gtk_widget_get_height(Content);

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
        gtk_widget_set_visible(GTK_WIDGET(Toolbar), FALSE);
        gtk_window_set_decorated(GTK_WINDOW(Window), FALSE);
        gtk_window_fullscreen(GTK_WINDOW(Window));
    }
    else
    {
        gtk_window_unfullscreen(GTK_WINDOW(Window));
        gtk_window_set_decorated(GTK_WINDOW(Window), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(Toolbar), TRUE);
    }
}

void WindowKit::Window::Implementation::OnActivate(GApplication* application, gpointer userData)
{
    auto self = static_cast<Implementation*>(userData);

    self->Window = ADW_APPLICATION_WINDOW(adw_application_window_new(GTK_APPLICATION(self->Application)));

    g_signal_connect(self->Window, "close-request", G_CALLBACK(OnClose), self);

    gtk_window_set_default_size(GTK_WINDOW(self->Window), self->Width, self->Height);
    gtk_window_set_title(GTK_WINDOW(self->Window), self->Title);
    gtk_window_set_resizable(GTK_WINDOW(self->Window), self->Resizable);
    gtk_widget_set_visible(GTK_WIDGET(self->Window), true);

    self->Toolbar = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());
    self->Header = ADW_HEADER_BAR(adw_header_bar_new());

    adw_header_bar_set_show_start_title_buttons(self->Header, TRUE);
    adw_header_bar_set_show_end_title_buttons(self->Header, TRUE);

    adw_toolbar_view_add_top_bar(self->Toolbar, GTK_WIDGET(self->Header));

    self->Content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    adw_toolbar_view_set_content(self->Toolbar, self->Content);

    adw_application_window_set_content(self->Window, GTK_WIDGET(self->Toolbar));

    gtk_widget_set_visible(GTK_WIDGET(self->Toolbar), true);
    gtk_widget_set_visible(self->Content, true);

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