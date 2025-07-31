module;
#include <GLFW/glfw3.h>

#include <stdexcept>
export module gui;
export import :window;

namespace
{
    void callback(int error, const char *description)
    {
        throw std::runtime_error("GLFW Error " + std::to_string(error) + ": " + description);
    }

} // namespace

export namespace gui
{
    void init()
    {
        glfwSetErrorCallback(callback);
        if (!glfwInit())
        {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    void terminate()
    {
        glfwTerminate();
    }
} // namespace gui
