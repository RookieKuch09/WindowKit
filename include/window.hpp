#pragma once

#include <exception>
#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace WindowKit
{
    class Exception : public std::exception
    {
    public:
        explicit Exception(const std::string& message);

        const char* what() const noexcept override;

    private:
        std::string mMessage;
    };

    struct WindowClose
    {
    };

    struct WindowResize
    {
        unsigned int Width;
        unsigned int Height;
    };

    class Window;

    struct WindowDescriptor
    {
        const char* Title;

        unsigned int Width;
        unsigned int Height;

        bool Resizable;
        bool Fullscreen;
    };

    class Window
    {
    public:
        Window(const WindowDescriptor& descriptor);
        ~Window();

        void Create();
        void Update();

        bool& Fullscreen();

        template <typename EventT, typename... UserData>
        void SetCallback(void (*callback)(Window&, const EventT&, UserData&...), UserData&... userData);

        struct Implementation;

    private:
        void Initialise();

        template <typename T>
        void CallCallback(const T& data);

        const char* mTitle;

        unsigned int mWidth;
        unsigned int mHeight;

        bool mResizable;
        bool mFullscreen;

        Implementation* mImplementation;

        std::unordered_map<std::type_index, std::function<void(Window&, void*)>> mCallbacks;

        static bool mInitialised;
    };
}

#include "../inline/window.inl"