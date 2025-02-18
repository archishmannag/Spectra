module;

#include <GL/glew.h>

#include <vector>

export module opengl:index_buffer;

export namespace opengl
{
    class c_index_buffer
    {
    public:
        c_index_buffer(const std::vector<unsigned int> &data, unsigned int count);
        ~c_index_buffer();

        auto bind() const -> void;
        auto unbind() -> void;

        [[nodiscard]] auto get_count() const -> unsigned int
        {
            return m_count;
        }

    private:
        unsigned int m_renderer_id{};
        unsigned int m_count{};
    };
} // namespace opengl

// Implementation
namespace opengl
{
    c_index_buffer::c_index_buffer(const std::vector<unsigned int> &data, unsigned int count)
        : m_count(count)
    {
        glGenBuffers(1, &m_renderer_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data.data(), GL_STATIC_DRAW);
    }

    c_index_buffer::~c_index_buffer()
    {
        glDeleteBuffers(1, &m_renderer_id);
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
