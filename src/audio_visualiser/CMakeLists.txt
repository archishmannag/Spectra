find_package(glew REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(visualiser main.cpp)

target_link_libraries(visualiser PRIVATE
    GLEW::GLEW
    glfw
)

target_include_directories(visualiser PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)