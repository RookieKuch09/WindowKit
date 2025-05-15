# WindowKit

**WindowKit** is a C++ library built for the [Mosaic Engine](https://github.com/RookieKuch09/Mosaic-Engine), designed as a cross-platform window and input management library. It abstracts the platform-specific aspects of window management, allowing you to focus on the application logic. **WindowKit** automatically selects the most suitable platform backend at compile-time, based on the system you're building on.

## Features
- **Cross-Platform Support**: Supports Linux, macOS, and Windows natively.
- **Backend Abstraction**: Automatically selects the most suitable backend for your platform.
- **Event Management**: Uses event querying to handle inputs and window interruptions.
- **Rendering**: Supports integration with Vulkan, OpenGL, Metal, and DirectX for rendering.

## Backends
**WindowKit** supports multiple backends, including Win32 for Windows, Cocoa for macOS, and GTK3, GTK4, or Adwaita (GNOME-exclusive) on Linux. The backend is selected automatically during the compilation process. Once compiled, the backend cannot be changed without recompiling the library.

- **Windows**: Uses the Win32 API.
- **macOS**: Uses the Cocoa framework.
- **Linux**: Supports **GTK3**, **GTK4**, and **Adwaita** (GNOME-exclusive).

## Rendering
**WindowKit** allows you to create and manage various rendering contexts, including **Vulkan**, **OpenGL**, **Metal**, and **DirectX**. However, note that some rendering APIs are exclusive to certain platforms:
- **Metal** is exclusive to macOS.
- **DirectX** is exclusive to Windows.
- **Vulkan** and **OpenGL** are cross-platform, though Vulkan may require specific setup for certain platforms.

## Installation

To install **WindowKit**, follow these steps:

1. Clone the repository:
    ```bash
    git clone https://github.com/RookieKuch09/WindowKit.git
    cd WindowKit
    ```

2. Create a build directory and navigate into it:
    ```bash
    mkdir build
    cd build
    ```

3. Run CMake to configure the project:
    ```bash
    cmake ..
    ```

4. Build the project:
    ```bash
    cmake --build .
    ```

5. Install the library system-wide (optional but recommended):
    ```bash
    sudo cmake --install .
    ```

## Usage Example

Here's a simple example demonstrating how to create a window using **WindowKit**:

```cpp
#include <WindowKit/WindowKit.hpp>

int main()
{
    try
    {
        WindowKit::WindowDescriptor windowDescriptor =
        {
            .Title = "New Window",
            .Width = 800,
            .Height = 600,
            .Resizable = false,
            .Fullscreen = false,
        };

    WindowKit::Window window(windowDescriptor);

    window.Create();

    bool updating = true;

    while (updating)
    {
        window.Update();

        const auto& events = window.QueryEvents();

        for (const auto& event : events)
        {
            if (event == WindowKit::Event::WindowClose)
            {
                updating = false;
            }
        }

        return 0;
    }
    catch (const WindowKit::Exception& error)
    {
        // Handle WindowKit error (WindowKit::Exception inherits std::exception, so similar usage)

        return 1;
    }
}
```

## Licensing

**WindowKit** is released under the MIT License. You can freely use, modify, and distribute the software, but if you use it in your own project, a credit would be greatly appreciated (but is not required).