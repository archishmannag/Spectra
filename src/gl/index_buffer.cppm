module;
#include <GL/glew.h>

#include <utility>
#include <vector>
export module opengl:index_buffer;

import utility;

export namespace opengl
{
    class c_index_buffer
    {
    public:
        c_index_buffer(const std::vector<unsigned int> &data, std::size_t count, GLenum usage = GL_STATIC_DRAW) noexcept;
        ~c_index_buffer();

        c_index_buffer(c_index_buffer &&other) noexcept;
        auto operator=(c_index_buffer &&other) noexcept -> c_index_buffer &;

        auto bind() const -> void;
        auto unbind() -> void;

        [[nodiscard]] auto get_count() const -> std::size_t;

        auto update_buffer(const std::vector<unsigned int> &data, std::size_t count) -> void;

    private:
        unsigned int m_ibo_id{};
        std::size_t m_count{};
        GLenum m_usage;
    };
} // namespace opengl

// Implementation
namespace opengl
{
    c_index_buffer::c_index_buffer(const std::vector<unsigned int> &data, std::size_t count, GLenum usage) noexcept
        : m_count(count),
          m_usage(usage)
    {
        auto init = [this, data]()
        {
            glGenBuffers(1, &m_ibo_id);
            if (m_count > 0)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_id);
                if (data.empty())
                {
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(unsigned int), nullptr, m_usage);
                }
                else
                {
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(unsigned int), data.data(), m_usage);
                }
            }
        };
        utility::c_notifier::subscribe(init);
    }

    c_index_buffer::~c_index_buffer()
    {
        glDeleteBuffers(1, &m_ibo_id);
    }

    c_index_buffer::c_index_buffer(c_index_buffer &&other) noexcept
        : m_ibo_id(std::exchange(other.m_ibo_id, 0)),
          m_count(std::exchange(other.m_count, 0)),
          m_usage(std::exchange(other.m_usage, GL_STATIC_DRAW))
    {
    }

    auto c_index_buffer::operator=(c_index_buffer &&other) noexcept -> c_index_buffer &
    {
        if (this != &other)
        {
            glDeleteBuffers(1, &m_ibo_id);
            m_ibo_id = std::exchange(other.m_ibo_id, 0);
            m_count = std::exchange(other.m_count, 0);
            m_usage = std::exchange(other.m_usage, GL_STATIC_DRAW);
        }
        return *this;
    }

    auto c_index_buffer::get_count() const -> std::size_t
    {
        return m_count;
    }

    auto c_index_buffer::update_buffer(const std::vector<unsigned int> &data, std::size_t count) -> void
    {
        bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data.data(), GL_STATIC_DRAW);
        m_count = count;
        unbind();
    }

    auto c_index_buffer::bind() const -> void
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_id);
    }

    auto c_index_buffer::unbind() -> void
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
} // namespace opengl
