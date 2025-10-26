module;
#include <GL/glew.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
export module opengl:shader;

import utility;
import glm;

namespace
{
    struct s_shader_program_source
    {
        std::string vertex_source;
        std::string fragment_source;
    };

    auto parse_file(const std::filesystem::path &path) noexcept -> s_shader_program_source
    {
        std::ifstream file(path);
        if (not file.is_open())
        {
            std::println(std::cerr, "Failed to open file: {}", path.string());
            return {};
        }

        enum class e_shader_type : std::int8_t
        {
            NONE = -1,
            VERTEX = 0,
            FRAGMENT = 1
        };
        using enum e_shader_type;

        std::string line;
        std::array<std::ostringstream, 2> streams;
        e_shader_type type = e_shader_type::NONE;
        while (std::getline(file, line))
        {
            if (line.find("#shader") != std::string::npos)
            {
                if (line.find("vertex") != std::string::npos)
                {
                    type = VERTEX;
                }
                else if (line.find("fragment") != std::string::npos)
                {
                    type = FRAGMENT;
                }
            }
            else
            {
                streams[static_cast<std::size_t>(type)] << line << '\n';
            }
        }

        return { .vertex_source = streams[0].str(), .fragment_source = streams[1].str() };
    }

    auto compile_shader(unsigned int type, const std::string &source) noexcept -> unsigned int
    {
        int result{};
        unsigned int shader_id = glCreateShader(type);
        const char *src = source.c_str();
        glShaderSource(shader_id, 1, &src, nullptr);
        glCompileShader(shader_id);

        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            int length{};
            glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
            char *message = static_cast<char *>(alloca(static_cast<unsigned long>(length) * sizeof(char)));
            glGetShaderInfoLog(shader_id, length, &length, message);
            std::println(std::cerr,
                         "Failed to compile {} shader\n"
                         "Error: {}",
                         type == GL_VERTEX_SHADER ? "vertex" : "fragment", message);
            glDeleteShader(shader_id);
            return 0;
        }

        return shader_id;
    }

    auto create_shader(const std::string &vertex_shader, const std::string &fragment_shader) noexcept -> unsigned int
    {
        unsigned program = glCreateProgram();
        unsigned vertex_shader_compiled = compile_shader(GL_VERTEX_SHADER, vertex_shader);
        unsigned fragment_shader_compiled = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

        glAttachShader(program, vertex_shader_compiled);
        glAttachShader(program, fragment_shader_compiled);
        glLinkProgram(program);
        glValidateProgram(program);

        glDeleteShader(vertex_shader_compiled);
        glDeleteShader(fragment_shader_compiled);

        return program;
    }

} // namespace

export namespace opengl
{
    class c_shader
    {
    public:
        /**
         * @brief Construct a new c_shader object
         *
         * @param shader_path - The path to the shader file
         * @note The shader file should contain both the vertex and fragment shaders
         */
        explicit c_shader(std::filesystem::path shader_source_file) noexcept(true);
        ~c_shader();

        c_shader(c_shader &&other) noexcept;
        auto operator=(c_shader &&other) noexcept -> c_shader &;

        auto bind() const -> void;
        auto unbind() const -> void;

        auto set_uniform_1f(const std::string &name, float value) -> void;
        auto set_uniform_1i(const std::string &name, int value) -> void;
        auto set_uniform_1ui(const std::string &name, unsigned int value) -> void;
        auto set_uniform_1fv(const std::string &name, const float *value, int count) -> void;
        auto set_uniform_1iv(const std::string &name, const int *value, int count) -> void;

        auto set_uniform_2f(const std::string &name, const glm::vec2 &value) -> void;
        auto set_uniform_3f(const std::string &name, const glm::vec3 &value) -> void;
        auto set_uniform_4f(const std::string &name, const glm::vec4 &value) -> void;

        auto set_uniform_mat4f(const std::string &name, const glm::mat4 &value) -> void;

    private:
        unsigned int m_shader_id{};
        std::unordered_map<std::string, int> m_uniform_location_cache;

        [[nodiscard]] auto get_uniform_location(const std::string &name) -> int;
    };
} // namespace opengl

// Implementation
namespace opengl
{
    c_shader::c_shader(std::filesystem::path shader_source_file) noexcept
    {
        auto init = [this, path = std::move(shader_source_file)]()
        {
            s_shader_program_source source = parse_file(path);
            m_shader_id = create_shader(source.vertex_source, source.fragment_source);
        };
        utility::c_notifier::subscribe(init);
    }

    c_shader::~c_shader()
    {
        glDeleteProgram(m_shader_id);
    }

    c_shader::c_shader(c_shader &&other) noexcept
        : m_shader_id(std::exchange(other.m_shader_id, 0)),
          m_uniform_location_cache(std::move(other.m_uniform_location_cache))
    {
    }

    auto c_shader::operator=(c_shader &&other) noexcept -> c_shader &
    {
        if (this != &other)
        {
            glDeleteProgram(m_shader_id);
            m_shader_id = std::exchange(other.m_shader_id, 0);
            m_uniform_location_cache = std::move(other.m_uniform_location_cache);
        }
        return *this;
    }

    auto c_shader::bind() const -> void
    {
        glUseProgram(m_shader_id);
    }

    auto c_shader::unbind() const -> void
    {
        glUseProgram(0);
    }

    auto c_shader::get_uniform_location(const std::string &name) -> int
    {
        if (m_uniform_location_cache.contains(name))
        {
            return m_uniform_location_cache[name];
        }
        int location = glGetUniformLocation(m_shader_id, name.c_str());
        if (location == -1)
        {
            std::println(std::cerr, "Warning: uniform '{}' doesn't exist!", name);
        }
        m_uniform_location_cache[name] = location;
        return location;
    }

    auto c_shader::set_uniform_1f(const std::string &name, float value) -> void
    {
        auto init = [this, &name, value]
        {
            bind();
            glUniform1f(get_uniform_location(name), value);
            unbind();
        };
        utility::c_notifier::subscribe(init);
    }

    auto c_shader::set_uniform_1i(const std::string &name, int value) -> void
    {
        auto init = [this, &name, value]
        {
            bind();
            glUniform1i(get_uniform_location(name), value);
            unbind();
        };
        utility::c_notifier::subscribe(init);
    }

    auto c_shader::set_uniform_1ui(const std::string &name, unsigned int value) -> void
    {
        auto init = [this, &name, value]
        {
            bind();
            glUniform1ui(get_uniform_location(name), value);
            unbind();
        };
        utility::c_notifier::subscribe(init);
    }

    auto c_shader::set_uniform_1fv(const std::string &name, const float *value, int count) -> void
    {
        auto init = [this, &name, value, count]
        {
            bind();
            glUniform1fv(get_uniform_location(name), count, value);
            unbind();
        };
        utility::c_notifier::subscribe(init);
    }

    auto c_shader::set_uniform_1iv(const std::string &name, const int *value, int count) -> void
    {
        auto init = [this, &name, value, count]
        {
            bind();
            glUniform1iv(get_uniform_location(name), count, value);
            unbind();
        };
        utility::c_notifier::subscribe(init);
    }

    auto c_shader::set_uniform_2f(const std::string &name, const glm::vec2 &value) -> void
    {
        auto init = [this, &name, value]
        {
            bind();
            glUniform2f(get_uniform_location(name), value.x, value.y);
            unbind();
        };
        utility::c_notifier::subscribe(init);
    }

    auto c_shader::set_uniform_3f(const std::string &name, const glm::vec3 &value) -> void
    {
        auto init = [this, &name, value]
        {
            bind();
            glUniform3f(get_uniform_location(name), value.x, value.y, value.z);
            unbind();
        };
        utility::c_notifier::subscribe(init);
    }

    auto c_shader::set_uniform_4f(const std::string &name, const glm::vec4 &value) -> void
    {
        auto init = [this, &name, value]
        {
            bind();
            glUniform4f(get_uniform_location(name), value.x, value.y, value.z, value.w);
            unbind();
        };
        utility::c_notifier::subscribe(init);
    }

    auto c_shader::set_uniform_mat4f(const std::string &name, const glm::mat4 &value) -> void
    {
        auto init = [this, &name, value]
        {
            bind();
            glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, glm::gtc::value_ptr(value));
            unbind();
        };
        utility::c_notifier::subscribe(init);
    }
} // namespace opengl
