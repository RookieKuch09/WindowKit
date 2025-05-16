#pragma once

#include <exception>
#include <string>

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

    enum class Event
    {
        WindowClose,
        WindowResize,
    };

    struct EventList
    {
        EventList();
        ~EventList();

        EventList(const EventList& other);
        EventList(EventList&& other) noexcept;

        EventList& operator=(const EventList& other);
        EventList& operator=(EventList&& other) noexcept;

        Event& operator[](unsigned int index);
        const Event& operator[](unsigned int index) const;

        Event* begin();
        Event* end();

        const Event* begin() const;
        const Event* end() const;

        unsigned int Size() const;

    private:
        void Append(Event event);
        void Purge();

        unsigned int mSize;
        unsigned int mOccupied;
        Event* mEvents;

        friend class Window;
    };

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

        const EventList& QueryEvents() const;

        bool& Fullscreen();

        struct Implementation;

    private:
        void Initialise();

        const char* mTitle;

        unsigned int mWidth;
        unsigned int mHeight;

        bool mResizable;
        bool mFullscreen;

        Implementation* mImplementation;

        EventList mEvents;

        static bool mInitialised;
    };
}