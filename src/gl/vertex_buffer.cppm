module;
#include <GL/glew.h>

#include <cstddef>
#include <vector>
export module opengl:vertex_buffer;

import utility;

export namespace opengl
{
    template <typename T>
    class c_vertex_buffer
    {
    public:
        c_vertex_buffer(const std::vector<T> &data, std::size_t count, unsigned int usage = GL_STATIC_DRAW);
        ~c_vertex_buffer();

        auto update_buffer(const std::vector<T> &data, std::size_t count, unsigned int offset = 0) -> void;

        auto bind() const -> void;
        auto unbind() const -> void;

    private:
        unsigned int m_vbo_id{};
        std::size_t m_count;
        GLenum m_usage;
    };
} // namespace opengl

// Implementation
namespace opengl
{
    template <typename T>
    c_vertex_buffer<T>::c_vertex_buffer(const std::vector<T> &data, std::size_t count, GLenum usage)
        : m_count(count),
          m_usage(usage)
    {
        auto init = [this, &data]()
        {
            glGenBuffers(1, &m_vbo_id);
            if (m_count > 0)
            {
                glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
                if (data.empty())
                {
                    glBufferData(GL_ARRAY_BUFFER, m_count * sizeof(T), nullptr, m_usage);
                }
                else
                {
                    glBufferData(GL_ARRAY_BUFFER, m_count * sizeof(T), data.data(), m_usage);
                }
            }
        };
        utility::c_notifier::subscribe(init);
    }

    template <typename T>
    c_vertex_buffer<T>::~c_vertex_buffer()
    {
        glDeleteBuffers(1, &m_vbo_id);
    }

    template <typename T>
    auto c_vertex_buffer<T>::update_buffer(const std::vector<T> &data, std::size_t count, unsigned int offset) -> void
    {
        bind();
        if (count > m_count)
        {
            m_count = count;
            glBufferData(GL_ARRAY_BUFFER, count * sizeof(T), data.data(), m_usage);
        }
        else
        {
            glBufferSubData(GL_ARRAY_BUFFER, offset, count * sizeof(T), data.data());
        }
        unbind();
    }

    template <typename T>
    auto c_vertex_buffer<T>::bind() const -> void
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
    }

    template <typename T>
    auto c_vertex_buffer<T>::unbind() const -> void
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
} // namespace opengl
