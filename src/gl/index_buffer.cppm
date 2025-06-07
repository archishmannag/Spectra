module;

#include <GL/glew.h>

#include <vector>

export module opengl:index_buffer;

export namespace opengl
{
    class c_index_buffer
    {
    public:
        c_index_buffer(const std::vector<unsigned int> &data, unsigned int count, GLenum usage = GL_STATIC_DRAW);
        ~c_index_buffer();

        auto bind() const -> void;
        auto unbind() -> void;

        [[nodiscard]] auto get_count() const -> unsigned int
        {
            return m_count;
        }

        auto update_buffer(const std::vector<unsigned int> &data, unsigned int count) -> void;

    private:
        unsigned int m_renderer_id{};
        unsigned int m_count{};
        GLenum m_usage;
    };
} // namespace opengl

// Implementation
namespace opengl
{
    c_index_buffer::c_index_buffer(const std::vector<unsigned int> &data, unsigned int count, GLenum usage)
        : m_count(count),
          m_usage(usage)
    {
        glGenBuffers(1, &m_renderer_id);
        if (count > 0)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data.data(), m_usage);
        }
    }

    c_index_buffer::~c_index_buffer()
    {
        glDeleteBuffers(1, &m_renderer_id);
    }

    auto c_index_buffer::update_buffer(const std::vector<unsigned int> &data, unsigned int count) -> void
    {
        bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data.data(), GL_STATIC_DRAW);
        m_count = count;
        unbind();
    }

    auto c_index_buffer::bind() const -> void
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);
    }

    auto c_index_buffer::unbind() -> void
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
} // namespace opengl
