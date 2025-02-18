module;

#include <GL/glew.h>

export module opengl:renderer;

import :shader;
import :vertex_array;
import :index_buffer;

export namespace opengl
{
    class c_renderer
    {
    public:
        auto draw(const c_vertex_array &varr, const c_index_buffer &ibuf, const c_shader &shader) const -> void;
        auto clear() const -> void;
    };
} // namespace opengl

// Implementation
namespace opengl
{
    auto c_renderer::draw(const c_vertex_array &varr, const c_index_buffer &ibuf, const c_shader &shader) const -> void
    {
        shader.bind();
        varr.bind();
        ibuf.bind();

        glDrawElements(GL_TRIANGLES, ibuf.get_count(), GL_UNSIGNED_INT, nullptr);
    }

    auto c_renderer::clear() const -> void
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
} // namespace opengl
