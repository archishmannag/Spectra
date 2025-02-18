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
        c_vertex_buffer(const std::vector<T> &data, unsigned int size);
        ~c_vertex_buffer();

        auto bind() const -> void;
        auto unbind() -> void;

    private:
        unsigned int m_renderer_id{};
    };
} // namespace opengl

// Implementation
namespace opengl
{
    template <typename T>
    c_vertex_buffer::c_vertex_buffer(const std::vector<T> &data, unsigned int size)
    {
        glGenBuffers(1, &m_renderer_id);
        glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
        glBufferData(GL_ARRAY_BUFFER, size, data.data(), GL_STATIC_DRAW);
    }

    c_vertex_buffer::~c_vertex_buffer()
    {
        glDeleteBuffers(1, &m_renderer_id);
    }

    auto c_vertex_buffer::bind() const -> void
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
    }

    auto c_vertex_buffer::unbind() -> void
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
} // namespace opengl
