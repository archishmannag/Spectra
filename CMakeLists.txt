cmake_minimum_required(VERSION 3.28)

project(AudioVisualiser LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(freetype REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(SndFile REQUIRED)
find_package(miniaudio REQUIRED)
find_package(utf8cpp REQUIRED)
find_package(OpenGL REQUIRED)

add_compile_options(
    -DSOURCE_DIR="${CMAKE_SOURCE_DIR}"
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wno-unknown-pragmas
    -fsanitize=address,undefined,leak
    -fno-omit-frame-pointer
    -fcolor-diagnostics # Clang-specific: color output
)
add_link_options(-fsanitize=address,undefined,leak)

add_subdirectory(src)
