module;
#include <GL/glew.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <numeric>
#include <ranges>
#include <variant>
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

    public:
        c_rectangle(glm::vec2 position, glm::vec2 size, glm::vec4 color);
        c_rectangle(c_rectangle &&other) noexcept = default;
        auto operator=(c_rectangle &&other) noexcept -> c_rectangle & = default;
        auto draw(const c_renderer &renderer, const glm::mat4 &projection) const -> void;
    };

    class c_circle
    {
        c_mesh m_mesh;

    public:
        c_circle(glm::vec2 position, float radius, glm::vec4 color);
        c_circle(c_circle &&other) noexcept = default;
        auto operator=(c_circle &&other) noexcept -> c_circle & = default;
        auto draw(const c_renderer &renderer, const glm::mat4 &projection) const -> void;
    };

    class c_triangle
    {
        c_mesh m_mesh;

    public:
        c_triangle(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, glm::vec4 color);
        c_triangle(c_triangle &&other) noexcept = default;
        auto operator=(c_triangle &&other) noexcept -> c_triangle & = default;
        auto draw(const c_renderer &renderer, const glm::mat4 &projection) const -> void;
    };

    class c_line
    {
        c_mesh m_mesh;

    public:
        c_line(glm::vec2 start, glm::vec2 end, glm::vec4 color);
        c_line(c_line &&other) noexcept = default;
        auto operator=(c_line &&other) noexcept -> c_line & = default;
        auto draw(const c_renderer &renderer, const glm::mat4 &projection) const -> void;
    };

    class c_ring
    {
        c_mesh m_mesh;

    public:
        c_ring(glm::vec2 position, float radius, float thickness, glm::vec2 angles, glm::vec4 color);
        c_ring(glm::vec2 position, float radius, float thickness, glm::vec4 color);
        c_ring(c_ring &&other) noexcept = default;
        auto operator=(c_ring &&other) noexcept -> c_ring & = default;
        auto draw(const c_renderer &renderer, const glm::mat4 &projection) const -> void;
    };

    class c_rounded_rectangle
    {
        c_mesh m_mesh;

    public:
        c_rounded_rectangle(glm::vec2 position, glm::vec2 size, glm::vec4 radius, glm::vec4 color);
        c_rounded_rectangle(glm::vec2 position, glm::vec2 size, float radius, glm::vec4 color);
        c_rounded_rectangle(c_rounded_rectangle &&other) noexcept = default;
        auto operator=(c_rounded_rectangle &&other) noexcept -> c_rounded_rectangle & = default;
        auto draw(const c_renderer &renderer, const glm::mat4 &projection) const -> void;
    };

    using variant = std::variant<c_rectangle, c_circle, c_triangle, c_line, c_rounded_rectangle>;
} // namespace opengl::shapes

// Implementation
namespace
{
    auto shape_shader() -> opengl::c_shader &
    {
        static opengl::c_shader shader{ SOURCE_DIR "/src/shaders/shape_shader.glsl" };
        return shader;
    }
} // namespace

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

    auto c_rectangle::draw(const c_renderer &renderer, const glm::mat4 &projection) const -> void
    {
        shape_shader().set_uniform_mat4f("projection", projection);

        m_mesh.draw(renderer, shape_shader());
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

    auto c_circle::draw(const c_renderer &renderer, const glm::mat4 &projection) const -> void
    {
        shape_shader().set_uniform_mat4f("projection", projection);

        m_mesh.draw(renderer, shape_shader());
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

    auto c_triangle::draw(const c_renderer &renderer, const glm::mat4 &projection) const -> void
    {
        shape_shader().set_uniform_mat4f("projection", projection);

        m_mesh.draw(renderer, shape_shader());
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

    auto c_line::draw(const c_renderer &renderer, const glm::mat4 &projection) const -> void
    {
        shape_shader().set_uniform_mat4f("projection", projection);

        m_mesh.draw(renderer, shape_shader());
    }

    c_rounded_rectangle::c_rounded_rectangle(glm::vec2 position, glm::vec2 size, glm::vec4 radius, glm::vec4 color)
        : m_mesh(std::vector<s_vertex>(), std::vector<unsigned int>(), e_render_primitive::triangle_fan)
    {
        std::vector<s_vertex> vertices;
        std::vector<unsigned int> indices;

        float bottom_left_radius = radius[0];
        float bottom_right_radius = radius[1];
        float top_right_radius = radius[2];
        float top_left_radius = radius[3];

        const int segments = 8; // Number of segments per corner
        // Center
        vertices.push_back({ .position = { position.x + (size.x / 2.0F), position.y + (size.y / 2.0F), 0.0F }, .color = color });
        for (int corner = 0; corner < 4; ++corner)
        {
            float corner_radius = 0.0F;
            glm::vec2 center;
            float start_angle = 0.0F;

            switch (corner)
            {
            case 0: // Bottom-left
                corner_radius = bottom_left_radius;
                center = { position.x + corner_radius, position.y + corner_radius };
                start_angle = std::numbers::pi_v<float>;
                break;
            case 1: // Bottom-right
                corner_radius = bottom_right_radius;
                center = { position.x + size.x - corner_radius, position.y + corner_radius };
                start_angle = 1.5F * std::numbers::pi_v<float>;
                break;
            case 2: // Top-right
                corner_radius = top_right_radius;
                center = { position.x + size.x - corner_radius, position.y + size.y - corner_radius };
                start_angle = 0.0F;
                break;
            case 3: // Top-left
                corner_radius = top_left_radius;
                center = { position.x + corner_radius, position.y + size.y - corner_radius };
                start_angle = 0.5F * std::numbers::pi_v<float>;
                break;
            default:
                [[unlikely]] break;
            }

            for (int i = 0; i <= segments; ++i)
            {
                float angle = start_angle + ((static_cast<float>(i) / static_cast<float>(segments)) * (std::numbers::pi_v<float> / 2.0F));
                float x = center.x + (corner_radius * std::cos(angle));
                float y = center.y + (corner_radius * std::sin(angle));
                vertices.push_back({ .position = { x, y, 0.0F }, .color = color });
            }
        }
        vertices.emplace_back(vertices[1].position, color); // Close the shape
        indices.resize(vertices.size());
        std::ranges::iota(indices, 0U);

        utility::c_notifier::subscribe([this, vertices = std::move(vertices), indices = std::move(indices)] mutable
                                       { m_mesh.update_mesh(std::move(vertices), std::move(indices)); });
    }

    c_rounded_rectangle::c_rounded_rectangle(glm::vec2 position, glm::vec2 size, float radius, glm::vec4 color)
        : c_rounded_rectangle(position, size, glm::vec4{ radius, radius, radius, radius }, color)
    {
    }

    auto c_rounded_rectangle::draw(const c_renderer &renderer, const glm::mat4 &projection) const -> void
    {
        shape_shader().set_uniform_mat4f("projection", projection);
        m_mesh.draw(renderer, shape_shader());
    }

    c_ring::c_ring(glm::vec2 position, float radius, float thickness, glm::vec2 angles, glm::vec4 color)
        : m_mesh(std::vector<s_vertex>(), std::vector<unsigned int>(), e_render_primitive::triangle_strip)
    {
        std::vector<s_vertex> vertices;
        std::vector<unsigned int> indices;

        const int segments = 64; // Number of segments to approximate the ring
        float start_angle = angles.x;
        float end_angle = angles.y;
        float angle_range = end_angle - start_angle;

        for (int i = 0; i <= segments; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(segments);
            float angle = start_angle + (t * angle_range);
            float cos_angle = std::cos(angle);
            float sin_angle = std::sin(angle);

            // Outer vertex
            float outer_x = position.x + (radius * cos_angle);
            float outer_y = position.y + (radius * sin_angle);
            vertices.push_back({ .position = { outer_x, outer_y, 0.0F }, .color = color });

            // Inner vertex
            float inner_x = position.x + ((radius - thickness) * cos_angle);
            float inner_y = position.y + ((radius - thickness) * sin_angle);
            vertices.push_back({ .position = { inner_x, inner_y, 0.0F }, .color = color });
        }

        indices.resize(vertices.size());
        std::ranges::iota(indices, 0U);

        utility::c_notifier::subscribe([this, vertices = std::move(vertices), indices = std::move(indices)] mutable
                                       { m_mesh.update_mesh(std::move(vertices), std::move(indices)); });
    }

    c_ring::c_ring(glm::vec2 position, float radius, float thickness, glm::vec4 color)
        : c_ring(position, radius, thickness, glm::vec2{ 0.0F, 2.0F * std::numbers::pi_v<float> }, color)
    {
    }

    auto c_ring::draw(const c_renderer &renderer, const glm::mat4 &projection) const -> void
    {
        shape_shader().set_uniform_mat4f("projection", projection);

        m_mesh.draw(renderer, shape_shader());
    }
} // namespace opengl::shapes
