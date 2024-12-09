find_package(glew REQUIRED)

add_library(shader
    shaders.cpp
)

target_link_libraries(shader PRIVATE
    GLEW::GLEW
)

target_include_directories(shader PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)