module;
#include <algorithm>
#include <cmath>
#include <cstring>
#include <ranges>
#include <string>
#include <vector>
export module gui:waveform;

import :panel;
import music;
import math;
import opengl;
import glm;

export namespace gui
{
    class c_waveform_panel final : public c_panel
    {
    public:
        c_waveform_panel(glm::vec2 position, glm::vec2 size, music::c_audio_manager &audio_manager);

        auto update_waveform() -> void;
        auto render_content() const -> void override;
        auto set_projection(const glm::mat4 &proj) -> void;

        auto on_mouse_move(glm::vec2 mouse_position) -> void override
        {
            c_panel::on_mouse_move(mouse_position);
        };
        auto on_mouse_press(glm::vec2 mouse_position, e_mouse_button button) -> void override
        {
            c_panel::on_mouse_press(mouse_position, button);
        };
        auto on_mouse_release(glm::vec2 mouse_position, e_mouse_button button) -> void override
        {
            c_panel::on_mouse_release(mouse_position, button);
        };
        auto on_resize(float width, float height) -> void override
        {
            c_panel::on_resize(width, height);
        };
        auto on_mouse_scroll(glm::vec2 position, glm::vec2 scroll_delta) -> void override
        {
            c_panel::on_mouse_scroll(position, scroll_delta);
        };

    private:
        music::c_audio_manager &m_audio_manager;

        float m_max_intensity{};
        std::vector<float> m_audio_samples;
        std::vector<float> m_fft;
        std::vector<opengl::s_vertex> m_vertices;
        std::vector<unsigned int> m_indices;
        opengl::c_mesh m_mesh;
        opengl::c_shader m_shader;
    };
} // namespace gui

// Implementation
namespace gui
{
    c_waveform_panel::c_waveform_panel(glm::vec2 position, glm::vec2 size, music::c_audio_manager &audio_manager)
        : c_panel(position, size, "Waveform Panel"),
          m_audio_manager(audio_manager),
          m_mesh(m_vertices, m_indices),
          m_shader(SOURCE_DIR "/src/shaders/frequency_shader.glsl")
    {
        m_audio_samples.resize(1U << 12U);
    }

    auto c_waveform_panel::update_waveform() -> void
    {
        if (not m_audio_manager.is_playing())
        {
            return; // No audio is playing, nothing to update
        }
        std::vector<float> current_frame_audio_samples = m_audio_manager.output_buffer();
        if (current_frame_audio_samples.empty())
        {
            return; // No audio samples to process
        }
        auto count_to_skip = m_audio_samples.size() - current_frame_audio_samples.size();
        std::memmove(m_audio_samples.data(), m_audio_samples.data() + count_to_skip, current_frame_audio_samples.size() * sizeof(float));
        std::memmove(m_audio_samples.data() + count_to_skip, current_frame_audio_samples.data(), current_frame_audio_samples.size() * sizeof(float));

        m_fft = math::fft(m_audio_samples)
                | std::views::transform([](auto &&datum)
                                        { return std::abs(datum.real()); })
                | std::ranges::to<std::vector>();
        m_max_intensity = std::max(std::ranges::max(m_fft), 1.F);

        auto base_freq = 20.F;
        float freq = base_freq;
        auto count = static_cast<int>(std::floor(12 * (std::log2(static_cast<float>(m_fft.size())) - std::log2(base_freq)))) + 1; // +1 to include the base frequency

        m_vertices.clear();
        m_indices.clear();
        m_vertices.resize(count * 4);
        m_indices.resize(count * 6);

        auto content_location = get_location();
        auto content_size = get_content_area_size();

        for (auto index = 0; freq < static_cast<float>(m_fft.size()); ++index)
        {
            auto value = m_fft[static_cast<std::size_t>(std::floor(freq))];
            auto x_base = content_location.x + (content_size.x * 2.5 / 100) + ((content_size.x * 95.F / 100) / count * index);
            auto y_base = content_location.y + (content_size.y * 2.5 / 100) + ((content_size.y * 95.F / 100) / m_max_intensity * value);
            m_vertices[(std::size_t)(index * 4) + 0] = opengl::s_vertex{ .position = glm::vec3{ x_base, content_location.y + (content_size.y * 2.5 / 100), 0 }, .color = { 1.F, 0.F, 0.F, 1.F } };
            m_vertices[(std::size_t)(index * 4) + 1] = opengl::s_vertex{ .position = glm::vec3{ (x_base + (content_size.x / count)), content_location.y + (content_size.y * 2.5 / 100), 0 }, .color = { 1.F, 0.F, 0.F, 1.F } };
            m_vertices[(std::size_t)(index * 4) + 2] = opengl::s_vertex{ .position = glm::vec3{ x_base, y_base, 0 }, .color = { 1.F, 0.F, 0.F, 1.F } };
            m_vertices[(std::size_t)(index * 4) + 3] = opengl::s_vertex{ .position = glm::vec3{ (x_base + (content_size.x / count)), y_base, 0 }, .color = { 1.F, 0.F, 0.F, 1.F } };
            m_indices[(std::size_t)(index * 6) + 0] = (unsigned)index * 4 + 0;
            m_indices[(std::size_t)(index * 6) + 1] = (unsigned)index * 4 + 1;
            m_indices[(std::size_t)(index * 6) + 2] = (unsigned)index * 4 + 2;
            m_indices[(std::size_t)(index * 6) + 3] = (unsigned)index * 4 + 2;
            m_indices[(std::size_t)(index * 6) + 4] = (unsigned)index * 4 + 1;
            m_indices[(std::size_t)(index * 6) + 5] = (unsigned)index * 4 + 3;
            freq *= static_cast<float>(std::pow(2, 1.F / 12.F));
        }
        m_mesh.update_mesh(m_vertices, m_indices);
    }

    auto c_waveform_panel::render_content() const -> void
    {
        m_mesh.draw(renderer(), m_shader);
    }

    auto c_waveform_panel::set_projection(const glm::mat4 &proj) -> void
    {
        m_shader.set_uniform_mat4f("projection", proj);
        set_projection_matrix(proj);
    }
} // namespace gui
