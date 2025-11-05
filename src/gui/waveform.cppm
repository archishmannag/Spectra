module;
#include <algorithm>
#include <cmath>
#include <complex>
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
        std::vector<opengl::shapes::c_rectangle> m_rectangles;
        opengl::c_shader m_shader;
    };
} // namespace gui

// Implementation
namespace gui
{
    c_waveform_panel::c_waveform_panel(glm::vec2 position, glm::vec2 size, music::c_audio_manager &audio_manager)
        : c_panel(position, size, "Waveform Panel"),
          m_audio_manager(audio_manager),
          m_shader(SOURCE_DIR "/src/shaders/frequency_shader.glsl")
    {
        m_audio_samples.resize(1U << 12U);
    }

    auto c_waveform_panel::update_waveform() -> void
    {
        std::vector<float> current_frame_audio_samples = m_audio_manager.output_buffer();
        if (current_frame_audio_samples.empty())
        {
            auto count_to_skip = 44100 / 60;
            std::memmove(m_audio_samples.data(), m_audio_samples.data() + count_to_skip, (m_audio_samples.size() - count_to_skip) * sizeof(float));
            std::memset(m_audio_samples.data() + (m_audio_samples.size() - count_to_skip), 0, count_to_skip * sizeof(float));
        }
        else
        {
            auto count_to_skip = m_audio_samples.size() - current_frame_audio_samples.size();
            std::memmove(m_audio_samples.data(), m_audio_samples.data() + count_to_skip, current_frame_audio_samples.size() * sizeof(float));
            std::memmove(m_audio_samples.data() + count_to_skip, current_frame_audio_samples.data(), current_frame_audio_samples.size() * sizeof(float));
        }

        math::helpers::hanning_window(m_audio_samples);
        m_fft = math::fft(m_audio_samples)
                | std::views::transform([](auto &&datum)
                                        { return std::max(0.F, std::log2(std::abs(datum))); })
                | std::ranges::to<std::vector>();
        m_max_intensity = std::ranges::max(m_fft) + 1.F;

        const float base_freq = 1.F;
        float freq = base_freq;
        float gamma = 2.F;
        auto freq_max = static_cast<float>(m_fft.size() / 2);
        auto count = static_cast<std::size_t>(12 * (std::log2(freq_max) - std::log2(base_freq))) + 1; // +1 to include the base frequency

        m_rectangles.clear();
        m_rectangles.reserve(count);

        auto content_location = get_location();
        auto content_size = get_content_area_size();

        for (auto index = 0; freq < freq_max; ++index)
        {
            float freq_low = freq;
            float freq_high = std::ceil(freq_max * std::pow(static_cast<float>(index + 1) / count, gamma));

            auto value = *std::max_element(m_fft.begin() + static_cast<long>(freq_low), std::min(m_fft.begin() + static_cast<long>(freq_high), m_fft.begin() + m_fft.size() / 2));
            auto x_base = content_location.x + (content_size.x * 2.5 / 100) + ((content_size.x * 95.F / 100) / count * index);
            auto y_base = content_location.y + (content_size.y * 2.5 / 100);

            m_rectangles.emplace_back(opengl::shapes::c_rectangle({ x_base, y_base },
                                                                  { content_size.x / count, (content_size.y * 95.F / 100) / m_max_intensity * value },
                                                                  { 0.F, 1.F, 0.F, 1.F }));
            freq = freq_high;
        }
    }

    auto c_waveform_panel::render_content() const -> void
    {
        for (const auto &rectangle : m_rectangles)
        {
            rectangle.draw(renderer(), get_projection_matrix());
        }
    }

    auto c_waveform_panel::set_projection(const glm::mat4 &proj) -> void
    {
        m_shader.set_uniform_mat4f("projection", proj);
        set_projection_matrix(proj);
    }
} // namespace gui
