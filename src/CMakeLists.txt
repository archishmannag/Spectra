add_subdirectory(audio)
add_subdirectory(gl)
add_subdirectory(glm)
add_subdirectory(math)

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
    FILES
        ${GLM_MODULES}
    PRIVATE
    FILE_SET math_modules TYPE CXX_MODULES
    FILES
        ${MATH_MODULES}
    PRIVATE
    FILE_SET audio_modules TYPE CXX_MODULES
    FILES
        ${AUDIO_MODULES}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        OpenGL::GL
        GLEW::GLEW
        glm::glm
        sfml-audio
        sfml-window
        sfml-system
)
