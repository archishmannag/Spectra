add_subdirectory(music)
add_subdirectory(gl)
add_subdirectory(math)
add_subdirectory(gui)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        test.cpp
    PRIVATE
    FILE_SET opengl_modules TYPE CXX_MODULES
    FILES
        ${OPENGL_MODULES}
    PRIVATE
    FILE_SET glm_modules TYPE CXX_MODULES # import glm instead of include
    BASE_DIRS
        ${glm_INCLUDE_DIRS}/glm
    FILES
        ${glm_INCLUDE_DIRS}/glm/glm.cppm
    PRIVATE
    FILE_SET math_modules TYPE CXX_MODULES
    FILES
        ${MATH_MODULES}
    PRIVATE
    FILE_SET music_modules TYPE CXX_MODULES
    FILES
        ${MUSIC_MODULES}
    FILE_SET gui_modules TYPE CXX_MODULES
    FILES
        ${GUI_MODULES}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Freetype::Freetype
        GLEW::GLEW
        glm::glm
        sfml-audio
        sfml-window
        sfml-system
        SndFile::sndfile
        miniaudio::miniaudio
        OpenGL::GL
)
