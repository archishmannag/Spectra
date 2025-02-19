module;
#include <GL/glew.h>

#include <print>
#include <string>

export module opengl:error;

export namespace opengl
{
    void gl_debug_callback_fn(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const void *data)
    {
        std::string i_source;
        std::string i_type;
        std::string i_severity;

        switch (source)
        {
        case GL_DEBUG_SOURCE_API:
            i_source = "API";
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            i_source = "WINDOW SYSTEM";
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            i_source = "SHADER COMPILER";
            break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
            i_source = "THIRD PARTY";
            break;

        case GL_DEBUG_SOURCE_APPLICATION:
            i_source = "APPLICATION";
            break;

        case GL_DEBUG_SOURCE_OTHER:
        default:
            i_source = "UNKNOWN";
            break;
        }

        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR:
            i_type = "ERROR";
            break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            i_type = "DEPRECATED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            i_type = "UDEFINED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_PORTABILITY:
            i_type = "PORTABILITY";
            break;

        case GL_DEBUG_TYPE_PERFORMANCE:
            i_type = "PERFORMANCE";
            break;

        case GL_DEBUG_TYPE_OTHER:
            i_type = "OTHER";
            break;

        case GL_DEBUG_TYPE_MARKER:
            i_type = "MARKER";
            break;

        default:
            i_type = "UNKNOWN";
            break;
        }

        switch (severity)
        {
        case GL_DEBUG_SEVERITY_HIGH:
            i_severity = "HIGH";
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            i_severity = "MEDIUM";
            break;

        case GL_DEBUG_SEVERITY_LOW:
            i_severity = "LOW";
            break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            i_severity = "NOTIFICATION";
            break;

        default:
            i_severity = "UNKNOWN";
            break;
        }

        std::println("{}: {} of {} severity, raised from {}: {}\n",
                     id, i_type, i_severity, i_source, msg);
    }
} // namespace opengl
