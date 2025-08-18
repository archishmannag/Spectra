module;
#include <GL/glew.h>

#include <vector>
export module opengl:mesh;

import :vertex_buffer;
import :index_buffer;
import :vertex_array;
import :buffer_layout;
import :shader;
import :renderer;
import utility;
import glm;

export namespace opengl
{
    struct s_vertex
    {
        glm::vec3 position;
        // glm::vec3 normal;
    };

    class c_mesh
    {
    private:
        std::vector<s_vertex> m_vertices;
        std::vector<unsigned int> m_indices;
        opengl::c_buffer_layout m_layout;

        c_vertex_array m_vertex_array;
        c_vertex_buffer<s_vertex> m_vertex_buffer;
        c_index_buffer m_index_buffer;

        e_render_primitive m_primitive_type = e_render_primitive::triangles;

    public:
        c_mesh(std::vector<s_vertex> vertices, std::vector<unsigned int> indices);

        /**
         * @brief  Update mesh data.
         * Note: Cannot update buffer layout, only data can be updated.
         * @param  vertices New vertex data
         * @param  indices New index data
         */
        void update_mesh(std::vector<s_vertex> vertices, std::vector<unsigned int> indices);
        void draw(const c_renderer &renderer, const c_shader &shader) const;
        void set_primitive_type(e_render_primitive type)
        {
            m_primitive_type = type;
        }
        [[nodiscard]] e_render_primitive get_primitive_type() const
        {
            return m_primitive_type;
        }
    };
} // namespace opengl

// Implementation
namespace opengl
{
    c_mesh::c_mesh(std::vector<s_vertex> vertices, std::vector<unsigned int> indices)
        : m_vertices(std::move(vertices)),
          m_indices(std::move(indices)),
          m_vertex_buffer(m_vertices, m_vertices.size(), GL_DYNAMIC_DRAW),
          m_index_buffer(m_indices, m_indices.size(), GL_DYNAMIC_DRAW)
    {
        auto init = [this]()
        {
            m_layout.push<float>(3); // Position
            // layout.push<float>(3); // Normal

            m_vertex_array.add_buffer(m_vertex_buffer, m_layout);
        };
        utility::c_notifier::subscribe(init);
    }

    void c_mesh::update_mesh(std::vector<s_vertex> vertices, std::vector<unsigned int> indices)
    {
        m_vertices = std::move(vertices);
        m_indices = std::move(indices);

        // Update buffers
        m_vertex_buffer.update_buffer(m_vertices, 0, m_vertices.size());
        m_index_buffer.update_buffer(m_indices, m_indices.size());
        m_vertex_array.add_buffer(m_vertex_buffer, m_layout);
    }

    void c_mesh::draw(const c_renderer &renderer, const c_shader &shader) const
    {
        renderer.clear();
        renderer.draw(m_vertex_array, m_vertex_buffer, m_index_buffer, shader, m_primitive_type);
    }
} // namespace opengl
