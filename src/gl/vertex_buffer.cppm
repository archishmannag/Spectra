module;
#include <GL/glew.h>

#include <vector>
export module opengl:vertex_buffer;

export namespace opengl
{
    class c_vertex_buffer
    {
    public:
        template <typename T>
        c_vertex_buffer(const std::vector<T> &data, unsigned int size, unsigned int usage = GL_STATIC_DRAW);
        ~c_vertex_buffer();

        template <typename T>
        auto update_buffer(const std::vector<T> &data, unsigned int offset, unsigned int size) -> void;

        auto bind() const -> void;
        auto unbind() const -> void;

    private:
        unsigned int m_vbo_id{};
        std::size_t m_size;
        GLenum m_usage;
    };
} // namespace opengl

// Implementation
namespace opengl
{
    template <typename T>
    c_vertex_buffer::c_vertex_buffer(const std::vector<T> &data, unsigned int size, GLenum usage)
        : m_size(size),
          m_usage(usage)
    {
        glGenBuffers(1, &m_vbo_id);
        if (size > 0)
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
            if (data.empty())
            {
                glBufferData(GL_ARRAY_BUFFER, size, nullptr, usage);
            }
            else
            {
                glBufferData(GL_ARRAY_BUFFER, size, data.data(), usage);
            }
        }
    }

    c_vertex_buffer::~c_vertex_buffer()
    {
        glDeleteBuffers(1, &m_vbo_id);
    }

    template <typename T>
    auto c_vertex_buffer::update_buffer(const std::vector<T> &data, unsigned int offset, unsigned int size) -> void
    {
        bind();
        if (size > m_size)
        {
            m_size = size;
            glBufferData(GL_ARRAY_BUFFER, size, data.data(), m_usage);
        }
        else
        {
            glBufferSubData(GL_ARRAY_BUFFER, offset, size, data.data());
        }
        unbind();
    }

    auto c_vertex_buffer::bind() const -> void
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
    }

    auto c_vertex_buffer::unbind() const -> void
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
} // namespace opengl
