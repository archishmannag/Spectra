cmake_minimum_required(VERSION 3.28)

project(AudioVisualiser LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(OpenGL REQUIRED)
find_package(SFML 2 REQUIRED COMPONENTS audio graphics window system)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

add_subdirectory(src)
