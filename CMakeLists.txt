cmake_minimum_required(VERSION 3.16)

# Set the compilers to use
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_OBJCXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define the project
project(WindowKit LANGUAGES CXX OBJCXX)

# Clear install directory to avoid stale files
install(CODE "
    message(STATUS \"Clearing old WindowKit install directory...\")
    file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/WindowKit\")
")

# Include required modules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Handle platform-specific backend logic
if(WIN32)
    message(STATUS "Detected Windows platform. Using Win32 backend.")
    set(WINDOWKIT_BACKEND "Win32")
    set(WINDOWKIT_DEFINES WINDOWKIT_BACKEND_WIN32)
    set(WINDOWKIT_LINK_LIBS user32 gdi32 opengl32)

elseif(APPLE)
    message(STATUS "Detected macOS platform. Using Cocoa backend.")
    set(WINDOWKIT_BACKEND "Cocoa")
    set(WINDOWKIT_DEFINES WINDOWKIT_BACKEND_COCOA)
    find_library(COCOA_FRAMEWORK Cocoa)
    set(WINDOWKIT_LINK_LIBS ${COCOA_FRAMEWORK})

else()
    # Linux platform, attempt to find suitable backends
    find_package(PkgConfig REQUIRED)

    # Try to find libadwaita first
    pkg_check_modules(ADWAITA QUIET libadwaita-1)
    if(ADWAITA_FOUND)
        message(STATUS "Detected libadwaita. Using Adwaita backend (${ADWAITA_VERSION}).")
        set(WINDOWKIT_BACKEND "Adwaita")
        set(WINDOWKIT_DEFINES WINDOWKIT_BACKEND_ADWAITA)
        set(WINDOWKIT_INCLUDE_DIRS ${ADWAITA_INCLUDE_DIRS})
        set(WINDOWKIT_LINK_LIBS ${ADWAITA_LIBRARIES})
        set(WINDOWKIT_CFLAGS ${ADWAITA_CFLAGS_OTHER})

    else()
        # If libadwaita is not found, try GTK4
        pkg_check_modules(GTK4 QUIET gtk4)
        if(GTK4_FOUND)
            message(STATUS "Falling back to GTK4 (${GTK4_VERSION}). Using GTK4 backend.")
            set(WINDOWKIT_BACKEND "GTK4")
            set(WINDOWKIT_DEFINES WINDOWKIT_BACKEND_GTK4)
            set(WINDOWKIT_INCLUDE_DIRS ${GTK4_INCLUDE_DIRS})
            set(WINDOWKIT_LINK_LIBS ${GTK4_LIBRARIES})
            set(WINDOWKIT_CFLAGS ${GTK4_CFLAGS_OTHER})

        else()
            message(FATAL_ERROR "No supported GUI backend found. Please install libadwaita, or GTK4")
        endif()
    endif()
endif()

message(STATUS "Configured WindowKit backend: ${WINDOWKIT_BACKEND}")

# Source files
file(GLOB_RECURSE SOURCES "source/**/*.cpp" "source/*.cpp" "source/**/*.mm")

# Create the library
add_library(WindowKit ${SOURCES})

# Set include directories
target_include_directories(WindowKit
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inline>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/WindowKit/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/WindowKit/inline>
)

# Platform-specific include dirs and compile flags
if(UNIX AND NOT APPLE)
    target_include_directories(WindowKit PUBLIC ${WINDOWKIT_INCLUDE_DIRS})
    target_compile_options(WindowKit PUBLIC ${WINDOWKIT_CFLAGS})
endif()

# Link dependencies
target_compile_definitions(WindowKit PUBLIC ${WINDOWKIT_DEFINES})
target_link_libraries(WindowKit PUBLIC ${WINDOWKIT_LINK_LIBS})

# Install targets and files
install(TARGETS WindowKit
    EXPORT WindowKitTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/WindowKit/include)
install(DIRECTORY inline/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/WindowKit/inline)

# Install primary interface files
install(DIRECTORY interface/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/WindowKit)

# Export the CMake config for consumers
install(EXPORT WindowKitTargets
    FILE WindowKitTargets.cmake
    NAMESPACE WindowKit::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/WindowKit
)

# Write package version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/WindowKitConfigVersion.cmake"
    VERSION 0.0.1
    COMPATIBILITY AnyNewerVersion
)

# Configure package version file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/WindowKitConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/WindowKitConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/WindowKit"
)

# Install config files for package
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/WindowKitConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/WindowKitConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/WindowKit
)

# Select Debug or Release compilation
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
else()
    add_compile_definitions(RELEASE)
endif()
