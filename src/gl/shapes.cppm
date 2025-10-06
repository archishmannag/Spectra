module;
#include <GL/glew.h>

#include <cmath>
#include <cstddef>
#include <numbers>
#include <numeric>
#include <vector>
export module opengl:shapes;

import :mesh;
import :shader;
import :renderer;

import glm;

export namespace opengl::shapes
{
    class c_rectangle
    {
        c_mesh m_mesh;
        c_shader m_shader{ SOURCE_DIR "/src/shaders/shape_shader.glsl" };

    public:
        c_rectangle(glm::vec2 position, glm::vec2 size, glm::vec4 color);
        auto draw(c_renderer &renderer, const glm::mat4 &projection) -> void;
    };

    class c_circle
    {
        c_mesh m_mesh;
        c_shader m_shader{ SOURCE_DIR "/src/shaders/shape_shader.glsl" };

    public:
        c_circle(glm::vec2 position, float radius, glm::vec4 color);
        auto draw(c_renderer &renderer, const glm::mat4 &projection) -> void;
    };

    class c_triangle
    {
        c_mesh m_mesh;
        c_shader m_shader{ SOURCE_DIR "/src/shaders/shape_shader.glsl" };

    public:
        c_triangle(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, glm::vec4 color);
        auto draw(c_renderer &renderer, const glm::mat4 &projection) -> void;
    };

    class c_line
    {
        c_mesh m_mesh;
        c_shader m_shader{ SOURCE_DIR "/src/shaders/shape_shader.glsl" };

    public:
        c_line(glm::vec2 start, glm::vec2 end, glm::vec4 color);
        auto draw(c_renderer &renderer, const glm::mat4 &projection) -> void;
    };
} // namespace opengl::shapes

// Implementation
namespace opengl::shapes
{
    c_rectangle::c_rectangle(glm::vec2 position, glm::vec2 size, glm::vec4 color)
        : m_mesh(std::vector<s_vertex>(), std::vector<unsigned int>())
    {
        std::vector<s_vertex> vertices(4);
        vertices[0] = { .position = { position.x, position.y, 0.0F }, .color = color };
        vertices[1] = { .position = { position.x + size.x, position.y, 0.0F }, .color = color };
        vertices[2] = { .position = { position.x + size.x, position.y + size.y, 0.0F }, .color = color };
        vertices[3] = { .position = { position.x, position.y + size.y, 0.0F }, .color = color };

        std::vector<unsigned int> indices = {
            0, 1, 2, // First triangle
            2, 3, 0  // Second triangle
        };

        utility::c_notifier::subscribe([this, vertices = std::move(vertices), indices = std::move(indices)] mutable
                                       { m_mesh.update_mesh(std::move(vertices), std::move(indices)); });
    }

    auto c_rectangle::draw(c_renderer &renderer, const glm::mat4 &projection) -> void
    {
        m_shader.set_uniform_mat4f("projection", projection);

        m_mesh.draw(renderer, m_shader);
    }

    c_circle::c_circle(glm::vec2 position, float radius, glm::vec4 color)
        : m_mesh(std::vector<s_vertex>(), std::vector<unsigned int>(), e_render_primitive::triangle_fan)
    {
        const int segments = 32; // Number of segments to approximate the circle
        std::vector<s_vertex> vertices(segments + 2);
        std::vector<unsigned int> indices(segments + 2);

        std::ranges::iota(indices, 0U);

        // Center vertex
        vertices[0] = { .position = { position.x, position.y, 0.0F }, .color = color };

        // Perimeter vertices
        for (std::size_t i = 0; i < segments; ++i)
        {
            float angle = 2.0F * std::numbers::pi_v<float> * static_cast<float>(i) / static_cast<float>(segments);
            float pos_x = position.x + (radius * std::cos(angle));
            float pos_y = position.y + (radius * std::sin(angle));
            vertices[i + 1] = { .position = { pos_x, pos_y, 0.0F }, .color = color };
        }
        vertices[segments + 1] = vertices[1]; // Close the circle

        utility::c_notifier::subscribe([this, vertices = std::move(vertices), indices = std::move(indices)] mutable
                                       { m_mesh.update_mesh(std::move(vertices), std::move(indices)); });
    }

    auto c_circle::draw(c_renderer &renderer, const glm::mat4 &projection) -> void
    {
        m_shader.set_uniform_mat4f("projection", projection);

        m_mesh.draw(renderer, m_shader);
    }

    c_triangle::c_triangle(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, glm::vec4 color)
        : m_mesh(std::vector<s_vertex>(), std::vector<unsigned int>())
    {
        std::vector<s_vertex> vertices(3);
        vertices[0] = { .position = { point1.x, point1.y, 0.0F }, .color = color };
        vertices[1] = { .position = { point2.x, point2.y, 0.0F }, .color = color };
        vertices[2] = { .position = { point3.x, point3.y, 0.0F }, .color = color };

        std::vector<unsigned int> indices = { 0, 1, 2 };

        utility::c_notifier::subscribe([this, vertices = std::move(vertices), indices = std::move(indices)] mutable
                                       { m_mesh.update_mesh(std::move(vertices), std::move(indices)); });
    }

    auto c_triangle::draw(c_renderer &renderer, const glm::mat4 &projection) -> void
    {
        m_shader.set_uniform_mat4f("projection", projection);

        m_mesh.draw(renderer, m_shader);
    }

    c_line::c_line(glm::vec2 start, glm::vec2 end, glm::vec4 color)
        : m_mesh(std::vector<s_vertex>(), std::vector<unsigned int>(), e_render_primitive::lines)
    {
        std::vector<s_vertex> vertices(2);
        vertices[0] = { .position = { start.x, start.y, 0.0F }, .color = color };
        vertices[1] = { .position = { end.x, end.y, 0.0F }, .color = color };

        std::vector<unsigned int> indices = { 0, 1 };

        utility::c_notifier::subscribe([this, vertices = std::move(vertices), indices = std::move(indices)] mutable
                                       { m_mesh.update_mesh(std::move(vertices), std::move(indices)); });
    }

    auto c_line::draw(c_renderer &renderer, const glm::mat4 &projection) -> void
    {
        m_shader.set_uniform_mat4f("projection", projection);

        m_mesh.draw(renderer, m_shader);
    }

} // namespace opengl::shapes
