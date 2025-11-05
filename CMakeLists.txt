cmake_minimum_required(VERSION 3.28)

project(AudioVisualiser LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(freetype)
find_package(glfw3)
find_package(miniaudio)
find_package(SndFile)
find_package(glew)
find_package(glm)
find_package(utf8cpp)
find_package(opengl_system)

# Detect compiler and apply flags accordingly
if (MSVC)
    # MSVC (Microsoft Visual C++)
    add_compile_options(
        /W4                     # Similar to -Wall -Wextra
        /permissive-            # Enforce standard conformance
        /EHsc                   # Enable C++ exceptions
        /Zi                     # Debug info
        /sdl                    # Enable additional security checks
        /D SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )

    add_link_options(
        /DEBUG                  # Include debug symbols
    )

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # GCC
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wno-unknown-pragmas
        -fno-omit-frame-pointer
        "-fsanitize=address,undefined,leak"
        -fdiagnostics-color=always
        -DSOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )

    add_link_options(
        "-fsanitize=address,undefined,leak"
    )

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Clang and AppleClang
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wno-unknown-pragmas
        -fno-omit-frame-pointer
        "-fsanitize=address,undefined,leak"
        -fcolor-diagnostics
        -DSOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )

    add_link_options(
        "-fsanitize=address,undefined,leak"
    )

endif()

add_subdirectory(src)
