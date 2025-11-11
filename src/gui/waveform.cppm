module;
#include <algorithm>
#include <chrono>
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
        std::vector<float> m_intensities;
        std::vector<opengl::shapes::c_rectangle> m_rectangles;
        std::vector<opengl::shapes::c_circle> m_circles;
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
        m_audio_samples.resize(1U << 13U);
    }

    auto c_waveform_panel::update_waveform() -> void
    {
        using math::helpers::operator""_percent;
        std::vector<float> current_frame_audio_samples = m_audio_manager.output_buffer()
                                                         | std::views::chunk(2)
                                                         | std::views::transform([](auto &&stereo_sample)
                                                                                 { return (static_cast<float>(stereo_sample[0]) + static_cast<float>(stereo_sample[1])) / 2.F; })
                                                         | std::ranges::to<std::vector>();
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
        auto temp = math::fft(m_audio_samples);
        auto fft = temp
                   | std::views::transform([](std::complex<float> &datum)
                                           { return std::abs(datum); })
                   | std::ranges::to<std::vector>();

        const float base_freq = 1.F;
        auto step = std::pow(2.F, 1.F / 12.F); // Semitone step
        m_max_intensity = 1.F;
        auto freq_max = static_cast<float>(fft.size()) / 2;

        auto prev = m_intensities;
        m_intensities.clear();
        m_intensities.reserve(prev.size());

        for (float freq = base_freq; freq < freq_max; freq = std::ceil(freq * step))
        {
            float next = std::ceil(freq * step);
            auto value = *std::max_element(fft.begin() + static_cast<long>(freq), std::min(fft.begin() + static_cast<long>(next), fft.begin() + freq_max));
            m_max_intensity = std::max(value, m_max_intensity);
            m_intensities.push_back(value);
        }

        // Normalize intensities
        for (auto &intensity : m_intensities)
        {
            intensity /= m_max_intensity;
        }

        static auto last_time = std::chrono::steady_clock::now();
        auto current_time = std::chrono::steady_clock::now();
        auto delta_time = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;

        const float smoothing_factor = 8.F;
        if (not prev.empty())
        {
            for (auto i = 0U; i < m_intensities.size(); i++)
            {
                m_intensities[i] = (m_intensities[i] - prev[i]) * smoothing_factor * static_cast<float>(delta_time) + prev[i];
            }
        }

        auto count = m_intensities.size();

        m_rectangles.clear();
        m_rectangles.reserve(count);
        m_circles.clear();
        m_circles.reserve(count);

        auto content_location = get_location();
        auto content_size = get_content_area_size();

        static int offset = 0;

        for (std::size_t index = 0; index < count; ++index)
        {
            float cell_width = content_size.x * 95._percent / count;
            auto value = m_intensities[index];
            auto x_base = content_location.x + (content_size.x * 2.5_percent) + (cell_width * index);
            auto y_base = content_location.y + (content_size.y * 2.5_percent);
            auto height = (content_size.y * 95._percent) * value * 9 / 10;
            glm::vec3 color_hsv = { (((index + offset) % count) / static_cast<float>(count)) * 360.F, .75F, 1.F };
            float radius = (std::sqrt(value) * 18) * 9 / 10;
            float max_width = cell_width * 75._percent;
            float min_width = cell_width * 15._percent;
            float rect_width = min_width + ((max_width - min_width) * (1.F - value));

            m_rectangles.emplace_back(opengl::shapes::c_rectangle({ x_base + ((cell_width - rect_width) / 2), y_base },
                                                                  { rect_width, height },
                                                                  math::helpers::hsv_to_rgba(color_hsv)));
            m_circles.emplace_back(opengl::shapes::c_circle({ x_base + (cell_width / 2), y_base + height },
                                                            radius,
                                                            math::helpers::hsv_to_rgba(color_hsv)));
        }
        offset = (offset + 1) % count;
    }

    auto c_waveform_panel::render_content() const -> void
    {
        opengl::c_renderer::set_scissor_area(get_location(), get_size());
        for (const auto &rectangle : m_rectangles)
        {
            rectangle.draw(renderer(), get_projection_matrix());
        }
        for (const auto &circle : m_circles)
        {
            circle.draw(renderer(), get_projection_matrix());
        }
        opengl::c_renderer::reset_scissor_area();
    }

    auto c_waveform_panel::set_projection(const glm::mat4 &proj) -> void
    {
        m_shader.set_uniform_mat4f("projection", proj);
        set_projection_matrix(proj);
    }
} // namespace gui
