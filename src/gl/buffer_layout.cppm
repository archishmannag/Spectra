module;
#include <GL/glew.h>

#include <vector>
export module opengl:buffer_layout;

export namespace opengl
{
    struct s_vertex_buffer_element
    {
        unsigned int type;
        unsigned int count;
        unsigned char normalized;

        [[nodiscard]] static auto get_size_of_type(unsigned int type) -> unsigned int
        {
            switch (type)
            {
            case GL_FLOAT:
                return sizeof(GLfloat);
            case GL_UNSIGNED_INT:
                return sizeof(GLuint);
            case GL_UNSIGNED_BYTE:
                return sizeof(GLubyte);
            default:
                return 0;
            }
        }
    };

    class c_buffer_layout
    {
    public:
        c_buffer_layout() noexcept = default;

        template <typename T>
        auto push(unsigned int /* count */) -> void
        {
            static_assert(false);
        }

        template <>
        auto push<float>(unsigned int count) -> void
        {
            m_elements.emplace_back(GL_FLOAT, count, GL_FALSE);
            m_stride += count * s_vertex_buffer_element::get_size_of_type(GL_FLOAT);
        }

        template <>
        auto push<unsigned int>(unsigned int count) -> void
        {
            m_elements.emplace_back(GL_UNSIGNED_INT, count, GL_FALSE);
            m_stride += count * s_vertex_buffer_element::get_size_of_type(GL_UNSIGNED_INT);
        }

        template <>
        auto push<unsigned char>(unsigned int count) -> void
        {
            m_elements.emplace_back(GL_UNSIGNED_BYTE, count, GL_TRUE);
            m_stride += count * s_vertex_buffer_element::get_size_of_type(GL_UNSIGNED_BYTE);
        }

        [[nodiscard]] auto get_stride() const -> unsigned int
        {
            return m_stride;
        }

        [[nodiscard]] auto get_elements() const -> const std::vector<s_vertex_buffer_element> &
        {
            return m_elements;
        }

    private:
        std::vector<s_vertex_buffer_element> m_elements;
        unsigned int m_stride{};
    };

} // namespace opengl
