#include "gl_abstraction/shaders.hpp"

#include <GL/glew.h>

namespace
{
    auto get_shader_type = [](ShaderType type)
    {
        switch (type)
        {
        case ShaderType::Vertex:
            return GL_VERTEX_SHADER;
        case ShaderType::Fragment:
            return GL_FRAGMENT_SHADER;
        }
        return GL_VERTEX_SHADER;
    };
} // namespace

bool compile_shader(const std::string &shader_data, ShaderType type)
{
    const auto shader_type = get_shader_type(type);

    const auto shader_id = glCreateShader(shader_type);
    if (not shader_id)
    {
        // TODO: Log error
        return false;
    }
    const auto cstr = shader_data.c_str();
    glShaderSource(shader_id, 1, &cstr, nullptr);
    glCompileShader(shader_id);

    return true;
}