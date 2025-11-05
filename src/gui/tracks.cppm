module;
#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <variant>
#include <vector>
export module gui:tracks;

import :panel;
import music;
import opengl;
import glm;

export namespace gui
{
    class c_track_panel final : public c_panel
    {
    public:
        c_track_panel(glm::vec2 position, glm::vec2 size, const std::vector<std::shared_ptr<music::c_track>> &tracks);
        ~c_track_panel() override = default;

        void render_content() const override;
        void set_projection(const glm::mat4 &proj);

        auto on_mouse_press(glm::vec2 mouse_position, e_mouse_button button) -> void override;
        auto on_mouse_release(glm::vec2 mouse_position, e_mouse_button button) -> void override;
        auto on_mouse_scroll(glm::vec2 position, glm::vec2 scroll_delta) -> void override;
        auto on_mouse_move(glm::vec2 mouse_position) -> void override;
        auto on_resize(float width, float height) -> void override;

    private:
        const std::vector<std::shared_ptr<music::c_track>> &m_tracks;
        glm::mat4 m_proj;

        // Layout constants
        float m_margin{ 20.F };
        float m_track_entry_height{ 60.F };
        float m_spacing{ 10.F };
        float m_button_radius{ 15.F };

        // Scrolling state
        mutable float m_scroll_offset{};
        mutable float m_max_scroll{};
    };
} // namespace gui

// Implementation
namespace gui
{
    c_track_panel::c_track_panel(glm::vec2 position, glm::vec2 size, const std::vector<std::shared_ptr<music::c_track>> &tracks)
        : c_panel(position, size, "Audio Tracks"),
          m_tracks(tracks),
          m_proj(glm::gtc::ortho(0.0F, size.x, 0.0F, size.y, -1.0F, 1.0F))
    {
    }

    void c_track_panel::render_content() const
    {
        if (is_minimized())
        {
            return;
        }

        glm::vec2 content_position = get_location();
        glm::vec2 content_size = get_content_area_size();

        if (content_size.x <= 0.0F || content_size.y <= 0.0F)
        {
            return;
        }

        // Clear previous text submissions for clipping to work correctly
        opengl::c_text_renderer::instance().draw_texts();
        opengl::c_renderer::set_scissor_area(content_position, content_size);

        float total_content_height = (static_cast<float>(m_tracks.size()) * (m_track_entry_height + m_spacing)) - m_spacing + (2.0F * m_margin);

        m_max_scroll = std::max(0.0F, total_content_height - content_size.y);
        m_scroll_offset = std::clamp(m_scroll_offset, 0.0F, m_max_scroll);

        float current_y = (content_position.y + content_size.y) - m_margin - m_track_entry_height + m_scroll_offset;

        std::vector<opengl::shapes::variant> shape_batch;
        for (const auto &track : m_tracks)
        {
            if (!track)
            {
                continue;
            }

            // Track entry background box
            glm::vec2 entry_position = { content_position.x + m_margin, current_y };
            glm::vec2 entry_size = { content_size.x - (m_margin * 2.0F), m_track_entry_height };

            glm::vec4 bg_color = { 0.15F, 0.15F, 0.2F, 0.9F };
            shape_batch.emplace_back(opengl::shapes::c_rectangle(entry_position, entry_size, bg_color));

            // Border around the entry
            glm::vec4 border_color = { 0.4F, 0.4F, 0.5F, 1.0F };
            float border_width = 2.0F;

            // Top border
            float top_border_y = entry_position.y + entry_size.y - border_width;
            shape_batch.emplace_back(opengl::shapes::c_rectangle({ entry_position.x, top_border_y }, { entry_size.x, border_width }, border_color));

            // Bottom border
            shape_batch.emplace_back(opengl::shapes::c_rectangle(entry_position, { entry_size.x, border_width }, border_color));

            // Left border
            shape_batch.emplace_back(opengl::shapes::c_rectangle(entry_position, { border_width, entry_size.y }, border_color));

            // Right border
            float right_border_x = entry_position.x + entry_size.x - border_width;
            shape_batch.emplace_back(opengl::shapes::c_rectangle({ right_border_x, entry_position.y }, { border_width, entry_size.y }, border_color));

            // Play button (circular)
            glm::vec2 button_center = { entry_position.x + m_margin + m_button_radius, entry_position.y + (entry_size.y / 2.0F) };
            glm::vec4 button_color = track->is_playing() ? glm::vec4{ 0.2F, 0.8F, 0.2F, 1.0F }  // Green
                                                         : glm::vec4{ 0.6F, 0.6F, 0.6F, 1.0F }; // Gray
            shape_batch.emplace_back(opengl::shapes::c_circle(button_center, m_button_radius, button_color));

            // Play symbol (triangle) or pause symbol (two rectangles)
            glm::vec4 symbol_color = { 1.0F, 1.0F, 1.0F, 1.0F };
            if (track->is_playing())
            {
                // Pause symbol
                float rect_width = 4.0F;
                float rect_height = 12.0F;
                glm::vec2 pause_left = button_center + glm::vec2(-6.0F, -rect_height / 2.0F);
                glm::vec2 pause_right = button_center + glm::vec2(2.0F, -rect_height / 2.0F);

                shape_batch.emplace_back(opengl::shapes::c_rectangle(pause_left, { rect_width, rect_height }, symbol_color));
                shape_batch.emplace_back(opengl::shapes::c_rectangle(pause_right, { rect_width, rect_height }, symbol_color));
            }
            else
            {
                // Play symbol
                float triangle_size = 8.0F;
                glm::vec2 tri_p1 = button_center + glm::vec2(-triangle_size * 0.5F, -triangle_size * 0.6F);
                glm::vec2 tri_p2 = button_center + glm::vec2(-triangle_size * 0.5F, triangle_size * 0.6F);
                glm::vec2 tri_p3 = button_center + glm::vec2(triangle_size * 0.7F, 0.0F);

                shape_batch.emplace_back(opengl::shapes::c_triangle(tri_p1, tri_p2, tri_p3, symbol_color));
            }

            // Track name text (moved up for better proportional placement)
            glm::vec2 text_pos = { button_center.x + m_button_radius + m_margin, entry_position.y + (entry_size.y / 2.0F) + 4.0F };
            std::string track_name = track->get_filename();

            auto text_size = opengl::c_text_renderer::instance().get_size(track_name, 1.0F);
            float available_width = entry_size.x - (button_center.x + m_button_radius + m_margin * 2.0F - entry_position.x);

            if (text_size.x > available_width)
            {
                // Truncate and add "..."
                float ratio = available_width / text_size.x;
                std::size_t new_length = static_cast<std::size_t>(std::max(0.0F, std::floor(static_cast<float>(track_name.length()) * ratio) - 3.0F)); // length * ratio - 3 for "..."
                track_name = track_name.substr(0, new_length) + "...";
            }

            glm::vec3 text_color = { 0.9F, 0.9F, 0.9F };
            opengl::c_text_renderer::instance().submit(track_name, text_pos, 1.0F, text_color);

            // Progress bar section
            float progress_bar_height = 8.0F;
            float progress_text_margin = 10.0F;

            // Calculate progress
            std::uint64_t current_frame = track->get_cursor_frame();
            std::uint64_t total_frames = track->get_total_frames();
            float progress = (total_frames > 0) ? static_cast<float>(current_frame) / static_cast<float>(total_frames) : 0.0F;
            progress = std::clamp(progress, 0.0F, 1.0F);

            // Calculate time text to determine progress bar width
            constexpr float sample_rate = 44100.0F;
            float current_seconds = static_cast<float>(current_frame) / sample_rate;
            float total_seconds = static_cast<float>(total_frames) / sample_rate;

            int current_minutes = static_cast<int>(current_seconds) / 60;
            int current_secs = static_cast<int>(current_seconds) % 60;
            int total_minutes = static_cast<int>(total_seconds) / 60;
            int total_secs = static_cast<int>(total_seconds) % 60;

            std::string progress_text = std::to_string(current_minutes) + ":" + (current_secs < 10 ? "0" : "") + std::to_string(current_secs) + " / " + std::to_string(total_minutes) + ":" + (total_secs < 10 ? "0" : "") + std::to_string(total_secs);

            auto time_text_size = opengl::c_text_renderer::instance().get_size(progress_text, 0.8F);

            // Progress bar starts at track name x-position and ends before time text
            glm::vec2 progress_bg_pos = {
                button_center.x + m_button_radius + m_margin,
                entry_position.y + (entry_size.y / 2.0F) - 12.0F
            };
            glm::vec2 progress_bg_size = {
                entry_size.x - (progress_bg_pos.x - entry_position.x) - time_text_size.x - progress_text_margin - m_margin,
                progress_bar_height
            };
            glm::vec4 progress_bg_color = { 0.25F, 0.25F, 0.3F, 0.8F };
            shape_batch.emplace_back(opengl::shapes::c_rectangle(progress_bg_pos, progress_bg_size, progress_bg_color));

            // Progress bar fill (colored based on play state)
            if (progress > 0.0F)
            {
                glm::vec2 progress_fill_size = { progress_bg_size.x * progress, progress_bar_height };
                glm::vec4 progress_fill_color = track->is_playing()
                                                    ? glm::vec4{ 0.2F, 0.7F, 1.0F, 0.9F }  // Bright blue when playing
                                                    : glm::vec4{ 0.6F, 0.6F, 0.7F, 0.7F }; // Gray when paused
                shape_batch.emplace_back(opengl::shapes::c_rectangle(progress_bg_pos, progress_fill_size, progress_fill_color));
            }

            // Progress bar border for definition
            glm::vec4 progress_border_color = { 0.5F, 0.5F, 0.6F, 0.6F };
            float progress_border_width = 1.0F;

            // Top border
            shape_batch.emplace_back(opengl::shapes::c_rectangle(
                { progress_bg_pos.x, progress_bg_pos.y + progress_bar_height - progress_border_width },
                { progress_bg_size.x, progress_border_width },
                progress_border_color));

            // Bottom border
            shape_batch.emplace_back(opengl::shapes::c_rectangle(
                progress_bg_pos,
                { progress_bg_size.x, progress_border_width },
                progress_border_color));

            // Left border
            shape_batch.emplace_back(opengl::shapes::c_rectangle(
                progress_bg_pos,
                { progress_border_width, progress_bar_height },
                progress_border_color));

            // Right border
            shape_batch.emplace_back(opengl::shapes::c_rectangle(
                { progress_bg_pos.x + progress_bg_size.x - progress_border_width, progress_bg_pos.y },
                { progress_border_width, progress_bar_height },
                progress_border_color));

            // Progress text (current time / total time) - positioned at the right end
            glm::vec2 progress_text_pos = {
                progress_bg_pos.x + progress_bg_size.x + progress_text_margin,
                progress_bg_pos.y - 2.0F // Slightly offset for better alignment
            };

            glm::vec3 progress_text_color = { 0.7F, 0.7F, 0.8F };
            opengl::c_text_renderer::instance().submit(progress_text, progress_text_pos, 0.8F, progress_text_color);

            // Move to next track position
            current_y -= (m_track_entry_height + m_spacing);
        }
        for (auto &shape : shape_batch)
        {
            std::visit([this](auto &shape_obj)
                       { shape_obj.draw(renderer(), m_proj); },
                       shape);
        }
        opengl::c_text_renderer::instance().draw_texts();
        opengl::c_renderer::reset_scissor_area();
    }

    void c_track_panel::set_projection(const glm::mat4 &proj)
    {
        m_proj = proj;
        set_projection_matrix(proj);
    }

    auto c_track_panel::on_mouse_press(glm::vec2 mouse_position, e_mouse_button button) -> void
    {
        c_panel::on_mouse_press(mouse_position, button);

        // Only handle left mouse button clicks on play buttons
        if (button != e_mouse_button::left)
        {
            return;
        }

        glm::vec2 content_pos = get_location();
        glm::vec2 content_size = get_content_area_size();

        // Ensure we have a valid content area
        if (content_size.x <= 0.0F || content_size.y <= 0.0F)
        {
            return;
        }

        float current_y = content_pos.y + content_size.y - m_margin - m_track_entry_height + m_scroll_offset;

        for (const auto &track : m_tracks)
        {
            if (!track)
            {
                current_y -= (m_track_entry_height + m_spacing);
                continue;
            }

            // Check if this entry is visible (at least partially)
            if (current_y + m_track_entry_height < content_pos.y || current_y > content_pos.y + content_size.y)
            {
                current_y -= (m_track_entry_height + m_spacing);
                continue;
            }

            // Calculate entry position for current track
            glm::vec2 entry_position = { content_pos.x + m_margin, current_y };
            glm::vec2 entry_size = { content_size.x - (m_margin * 2.0F), m_track_entry_height };

            // Calculate play button position and check if click is within it
            glm::vec2 button_center = { content_pos.x + (2 * m_margin) + m_button_radius, current_y + (m_track_entry_height / 2.0F) };

            // Check if mouse click is within the circular play button
            float distance = std::hypot(mouse_position.x - button_center.x, mouse_position.y - button_center.y);

            if (distance <= m_button_radius)
            {
                // Toggle play/pause for this track
                if (track->is_playing())
                {
                    track->pause();
                }
                else
                {
                    track->play();
                }
                return; // Exit after handling the click
            }

            // Check if click is on progress bar for seeking
            float progress_bar_height = 8.0F;
            float progress_text_margin = 10.0F;

            // Calculate time text to determine progress bar width (same calculation as in render)
            std::uint64_t total_frames = track->get_total_frames();
            constexpr float sample_rate = 44100.0F;
            float total_seconds = static_cast<float>(total_frames) / sample_rate;
            int total_minutes = static_cast<int>(total_seconds) / 60;
            int total_secs = static_cast<int>(total_seconds) % 60;
            std::string time_text = "0:00 / " + std::to_string(total_minutes) + ":" + (total_secs < 10 ? "0" : "") + std::to_string(total_secs);
            auto time_text_size = opengl::c_text_renderer::instance().get_size(time_text, 0.8F);

            // Progress bar position and size (same calculation as in render)
            glm::vec2 progress_bg_pos = {
                button_center.x + m_button_radius + m_margin,
                entry_position.y + (entry_size.y / 2.0F) - 12.0F
            };
            glm::vec2 progress_bg_size = {
                entry_size.x - (progress_bg_pos.x - entry_position.x) - time_text_size.x - progress_text_margin - m_margin,
                progress_bar_height
            };

            // Check if click is within progress bar bounds
            if (mouse_position.x >= progress_bg_pos.x && mouse_position.x <= progress_bg_pos.x + progress_bg_size.x && mouse_position.y >= progress_bg_pos.y && mouse_position.y <= progress_bg_pos.y + progress_bg_size.y)
            {
                // Calculate seek position based on click x position
                float click_ratio = (mouse_position.x - progress_bg_pos.x) / progress_bg_size.x;
                click_ratio = std::clamp(click_ratio, 0.0F, 1.0F);

                // Calculate target frame
                auto target_frame = static_cast<std::uint64_t>(static_cast<float>(total_frames) * click_ratio);

                // Seek to target frame
                track->seek(target_frame);
            }

            // Move to next track position
            current_y -= (m_track_entry_height + m_spacing);
        }
    }

    auto c_track_panel::on_mouse_release(glm::vec2 mouse_position, e_mouse_button button) -> void
    {
        // Call base class implementation for panel functionality
        c_panel::on_mouse_release(mouse_position, button);

        // No additional logic needed for mouse release in track panel currently
        // This is here for future functionality like button visual feedback
    }

    auto c_track_panel::on_mouse_scroll(glm::vec2 position, glm::vec2 scroll_delta) -> void
    {
        // Call base class implementation first
        c_panel::on_mouse_scroll(position, scroll_delta);

        // Scroll sensitivity - negative scroll_delta.y means to scroll up (should move content down)
        float scroll_speed = 30.0F;
        m_scroll_offset -= scroll_delta.y * scroll_speed;

        // Clamp scroll offset to valid range
        m_scroll_offset = std::clamp(m_scroll_offset, 0.0F, m_max_scroll);
    }

    auto c_track_panel::on_mouse_move(glm::vec2 mouse_position) -> void
    {
        c_panel::on_mouse_move(mouse_position);
    }

    auto c_track_panel::on_resize(float width, float height) -> void
    {
        c_panel::on_resize(width, height);
    }
} // namespace gui
