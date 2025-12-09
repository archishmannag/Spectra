add_subdirectory(music)
add_subdirectory(gl)
add_subdirectory(math)
add_subdirectory(utility)
add_subdirectory(gui)

add_library(visualizer_lib)

add_executable(${PROJECT_NAME})

target_sources(visualizer_lib
    PUBLIC
    FILE_SET opengl_modules TYPE CXX_MODULES
    FILES
    ${OPENGL_MODULES}
    FILE_SET glm_modules TYPE CXX_MODULES # import glm instead of include
    BASE_DIRS
    ${glm_INCLUDE_DIRS}/glm
    FILES
    ${glm_INCLUDE_DIRS}/glm/glm.cppm
    FILE_SET math_modules TYPE CXX_MODULES
    FILES
    ${MATH_MODULES}
    FILE_SET music_modules TYPE CXX_MODULES
    FILES
    ${MUSIC_MODULES}
    FILE_SET utility_modules TYPE CXX_MODULES
    FILES
    ${UTILITY_MODULES}
    FILE_SET gui_modules TYPE CXX_MODULES
    FILES
    ${GUI_MODULES}
)

target_link_libraries(visualizer_lib
    PUBLIC
    Freetype::Freetype
    glfw
    miniaudio::miniaudio
    SndFile::sndfile
    GLEW::GLEW
    glm::glm
    utf8cpp
    opengl::opengl

    # stdc++exp # Link with the experimental C++ library for std::stacktrace on libstdc++ (Will remove for libcxx)
)

target_sources(${PROJECT_NAME}
    PRIVATE
    main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE visualizer_lib)
