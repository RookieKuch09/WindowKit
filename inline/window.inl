#pragma once

#include "../include/window.hpp"

template <typename T>
void WindowKit::Window::CallCallback(const T& data)
{
    auto key = std::type_index(typeid(T));

    if (mCallbacks.contains(key))
    {
        mCallbacks[key](*this, (void*)&data);
    }
}

template <typename EventT, typename... UserData>
void WindowKit::Window::SetCallback(void (*callback)(Window&, const EventT&, UserData&...), UserData&... userData)
{
    using CallbackType = std::function<void(Window&, const EventT&, UserData&...)>;

    auto wrapper = [callback, &userData...](Window& w, const EventT& e)
    {
        callback(w, e, userData...);
    };

    auto key = std::type_index(typeid(EventT));

    mCallbacks[key] = [wrapper](Window& w, const void* eventPtr)
    {
        wrapper(w, *static_cast<const EventT*>(eventPtr));
    };
}