module;

#include <GL/glew.h>

#include <cstdint>
#include <ranges>
#include <utility>

export module opengl:vertex_array;

import :vertex_buffer;
import :buffer_layout;

import utility;

export namespace opengl
{
    class c_vertex_array
    {
    public:
        c_vertex_array() noexcept;
        ~c_vertex_array();

        c_vertex_array(c_vertex_array &&other) noexcept;
        auto operator=(c_vertex_array &&other) noexcept -> c_vertex_array &;

        template <typename T>
        auto add_buffer(const c_vertex_buffer<T> &vbuff, const c_buffer_layout &layout) const -> void;

        auto bind() const -> void;
        auto unbind() const -> void;

    private:
        unsigned int m_vao_id{};
    };
} // namespace opengl

// Implementation
namespace opengl
{
    c_vertex_array::c_vertex_array() noexcept
    {
        auto init = [this]()
        {
            glGenVertexArrays(1, &m_vao_id);
        };
        utility::c_notifier::subscribe(init);
    }

    c_vertex_array::~c_vertex_array()
    {
        glDeleteVertexArrays(1, &m_vao_id);
    }

    c_vertex_array::c_vertex_array(c_vertex_array &&other) noexcept
        : m_vao_id(std::exchange(other.m_vao_id, 0))
    {
    }

    auto c_vertex_array::operator=(c_vertex_array &&other) noexcept -> c_vertex_array &
    {
        if (this != &other)
        {
            glDeleteVertexArrays(1, &m_vao_id);
            m_vao_id = std::exchange(other.m_vao_id, 0);
        }
        return *this;
    }

    template <typename T>
    auto c_vertex_array::add_buffer(const c_vertex_buffer<T> &vbuff, const c_buffer_layout &layout) const -> void
    {
        bind();
        vbuff.bind();
        for (std::uint64_t offset{}; const auto &[index, element] : layout.get_elements() | std::views::enumerate)
        {
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, element.count, element.type, element.normalized, layout.get_stride(), reinterpret_cast<const void *>(offset));
            offset += static_cast<std::uint64_t>(element.count) * s_vertex_buffer_element::get_size_of_type(element.type);
        }
        unbind();
    }

    auto c_vertex_array::bind() const -> void
    {
        glBindVertexArray(m_vao_id);
    }

    auto c_vertex_array::unbind() const -> void
    {
        glBindVertexArray(0);
    }
} // namespace opengl
