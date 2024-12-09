#include "gl_abstraction/shaders.hpp"

#include <GL/glew.h>

#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <format>
#include <iostream>
#include <memory>
#include <string>

namespace
{
    using WindowPtr = std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)>;

    void destroyWindow(GLFWwindow *window)
    {
        if (window)
            glfwDestroyWindow(window);
    }
    WindowPtr createWindow(int width, int height, const std::string &title)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        GLFWwindow *window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        return { window, destroyWindow };
    }

    void errorCallback(int error, const char *description)
    {
        std::cerr << std::format("Error: {0} - {1}\n", error, description);
    }
} // namespace

int main()
{
    if (not glfwInit())
    {
        std::cerr << "Failed to initialise GLFW\n";
        return -1;
    }
    glfwSetErrorCallback(errorCallback);

    auto my_window = createWindow(800, 600, "Audio Visualiser");

    if (not my_window)
    {
        std::cerr << "Failed to create window\n";
        return -1;
    }

    glfwMakeContextCurrent(my_window.get());

    if (not glewInit() == GLEW_OK)
    {
        std::cerr << "Failed to initialise GLEW\n";
        return -1;
    }

    compile_shader("(void main() {gl_Position = vec4(0.0, 0.0, 0.0, 1.0);})", ShaderType::Vertex);

    my_window.reset();
    glfwTerminate();
    return 0;
}