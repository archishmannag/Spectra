module;
#include <GL/glew.h>
export module opengl:renderer;

import :shader;
import :vertex_array;
import :index_buffer;
import :vertex_buffer;

export namespace opengl
{
    enum class e_render_primitive : std::uint8_t
    {
        triangles = GL_TRIANGLES,
        triangle_strip = GL_TRIANGLE_STRIP,
        triangle_fan = GL_TRIANGLE_FAN,
        lines = GL_LINES,
        line_strip = GL_LINE_STRIP,
        line_loop = GL_LINE_LOOP,
        points = GL_POINTS
    };

    class c_renderer
    {
    public:
        template <typename T>
        auto draw(
            const c_vertex_array &varr,
            const c_vertex_buffer<T> &vbuff,
            const c_index_buffer &ibuff,
            const c_shader &shader,
            e_render_primitive primitive = e_render_primitive::triangles) const -> void;
        static auto clear() -> void;
        static auto clear_color(float red, float green, float blue, float alpha = 1.0F) -> void;
    };
} // namespace opengl

// Implementation
namespace opengl
{
    template <typename T>
    auto c_renderer::draw(
        const c_vertex_array &varr,
        const c_vertex_buffer<T> &vbuff,
        const c_index_buffer &ibuff,
        const c_shader &shader,
        e_render_primitive primitive) const -> void
    {
        shader.bind();
        varr.bind();
        vbuff.bind();
        ibuff.bind();

        glDrawElements(static_cast<GLenum>(primitive), static_cast<int>(ibuff.get_count()), GL_UNSIGNED_INT, nullptr);
    }

    auto c_renderer::clear() -> void
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    auto c_renderer::clear_color(float red, float green, float blue, float alpha) -> void
    {
        glClearColor(red, green, blue, alpha);
    }
} // namespace opengl
