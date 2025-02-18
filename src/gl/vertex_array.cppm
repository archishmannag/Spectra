module;

#include <GL/glew.h>

#include <ranges>

export module opengl:vertex_array;

import :vertex_buffer;
import :buffer_layout;

export namespace opengl
{
    class c_vertex_array
    {
    public:
        c_vertex_array();
        ~c_vertex_array();

        auto add_buffer(const c_vertex_buffer &vbuff, const c_buffer_layout &layout) const -> void;

        auto bind() const -> void;
        auto unbind() -> void;

    private:
        unsigned int m_renderer_id{};
    };
} // namespace opengl

// Implementation
namespace opengl
{
    c_vertex_array::c_vertex_array()
    {
        glGenVertexArrays(1, &m_renderer_id);
    }

    c_vertex_array::~c_vertex_array()
    {
        glDeleteVertexArrays(1, &m_renderer_id);
    }

    auto c_vertex_array::add_buffer(const c_vertex_buffer &vbuff, const c_buffer_layout &layout) const -> void
    {
        bind();
        vbuff.bind();
        const auto &elements = layout.get_elements();
        unsigned int offset{};

        for (const auto &[index, element] : elements | std::views::enumerate)
        {
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, element.count, element.type, element.normalized, layout.get_stride(), reinterpret_cast<const void *>(offset));
            offset += element.count * s_vertex_buffer_element::get_size_of_type(element.type);
        }
    }

    auto c_vertex_array::bind() const -> void
    {
        glBindVertexArray(m_renderer_id);
    }

    auto c_vertex_array::unbind() -> void
    {
        glBindVertexArray(0);
    }
} // namespace opengl
