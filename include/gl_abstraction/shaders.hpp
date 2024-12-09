#ifndef SHADERS_HPP
#define SHADERS_HPP

#include <string>

enum class ShaderType
{
    Vertex,
    Fragment
};

bool compile_shader(const std::string &shader_data, ShaderType type);

#endif // SHADERS_HPP