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

add_compile_options(
    -DSOURCE_DIR="${CMAKE_SOURCE_DIR}"
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wno-unknown-pragmas
    -fno-omit-frame-pointer
    "-fsanitize=address,undefined,leak"
    -fcolor-diagnostics # Clang-specific: color output
)
add_link_options(
    "-fsanitize=address,undefined,leak"
    )

add_subdirectory(src)
