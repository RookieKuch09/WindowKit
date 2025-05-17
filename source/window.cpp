#include "../include/window.hpp"

WindowKit::Exception::Exception(const std::string& message)
    : mMessage(message)
{
}

const char* WindowKit::Exception::what() const noexcept
{
    return mMessage.c_str();
}

bool WindowKit::Window::mInitialised = false;

WindowKit::Window::Window(const WindowDescriptor& descriptor)
{
    mTitle = descriptor.Title;

    mFullscreen = descriptor.Fullscreen;
    mResizable = descriptor.Resizable;

    mWidth = descriptor.Width;
    mHeight = descriptor.Height;

    if (not mInitialised)
    {
        Initialise();

        mInitialised = true;
    }
}

bool& WindowKit::Window::Fullscreen()
{
    return mFullscreen;
}