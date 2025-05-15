#include "../include/window.hpp"

WindowKit::Exception::Exception(const std::string& message)
    : mMessage(message)
{
}

const char* WindowKit::Exception::what() const noexcept
{
    return mMessage.c_str();
}

WindowKit::EventList::EventList()
    : mEvents(nullptr), mSize(0), mOccupied(0)
{
}

WindowKit::EventList::~EventList()
{
    if (mEvents)
    {
        delete[] mEvents;
    }
}

WindowKit::EventList::EventList(const EventList& other)
{
    mSize = other.mSize;
    mOccupied = other.mOccupied;
    mEvents = new Event[mSize];

    for (unsigned int index = 0; index < mOccupied; index++)
    {
        mEvents[index] = other.mEvents[index];
    }
}

WindowKit::EventList::EventList(EventList&& other) noexcept
{
    mSize = other.mSize;
    mOccupied = other.mOccupied;
    mEvents = other.mEvents;

    other.mSize = 0;
    other.mOccupied = 0;
    other.mEvents = nullptr;
}

WindowKit::EventList& WindowKit::EventList::operator=(const EventList& other)
{
    if (this == &other)
    {
        return *this;
    }

    if (mEvents)
    {
        delete mEvents;
    }

    mSize = other.mSize;
    mOccupied = other.mOccupied;
    mEvents = new Event[mSize];

    for (unsigned int index = 0; index < mOccupied; index++)
    {
        mEvents[index] = other.mEvents[index];
    }

    return *this;
}

WindowKit::EventList& WindowKit::EventList::operator=(EventList&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    if (mEvents)
    {
        delete mEvents;
    }

    mSize = other.mSize;
    mOccupied = other.mOccupied;
    mEvents = other.mEvents;

    other.mSize = 0;
    other.mOccupied = 0;
    other.mEvents = nullptr;

    return *this;
}

WindowKit::Event& WindowKit::EventList::operator[](unsigned int index)
{
    if (index >= mOccupied)
    {
        throw WindowKit::Exception("Provided index for WindowKit::EventList::operator[] was out-of-range");
    }

    return mEvents[index];
}

const WindowKit::Event& WindowKit::EventList::operator[](unsigned int index) const
{
    if (index >= mOccupied)
    {
        throw WindowKit::Exception("Provided index for WindowKit::EventList::operator[] was out-of-range");
    }

    return mEvents[index];
}

WindowKit::Event* WindowKit::EventList::begin()
{
    return mEvents;
}

WindowKit::Event* WindowKit::EventList::end()
{
    return mEvents + mOccupied;
}

const WindowKit::Event* WindowKit::EventList::begin() const
{
    return mEvents;
}

const WindowKit::Event* WindowKit::EventList::end() const
{
    return mEvents + mOccupied;
}

unsigned int WindowKit::EventList::Size() const
{
    return mOccupied;
}

void WindowKit::EventList::Append(Event event)
{
    if (mSize == mOccupied)
    {
        unsigned int oldSize = mSize;

        if (mSize == 0)
        {
            mSize = 1;
        }

        mSize *= 2;

        Event* newEvents = new Event[mSize];

        for (unsigned int index = 0; index < mOccupied; index++)
        {
            newEvents[index] = mEvents[index];
        }

        delete[] mEvents;

        mEvents = newEvents;
    }

    mEvents[mOccupied] = event;

    mOccupied++;
}

void WindowKit::EventList::Purge()
{
    if (mEvents)
    {
        delete[] mEvents;
    }

    mSize = 0;
    mOccupied = 0;
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

const WindowKit::EventList& WindowKit::Window::QueryEvents() const
{
    return mEvents;
}

bool& WindowKit::Window::Fullscreen()
{
    return mFullscreen;
}