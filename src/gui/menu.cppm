module;
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
export module gui:menu;

import opengl;
import glm;
import utility;

export namespace gui
{
    class c_popup_menu
    {
    public:
        struct s_menu_item
        {
            std::string text;
            std::function<void()> callback;
            bool is_hovered = false;
            float width = 0.0F;
            float height = 0.0F;
        };

        enum class e_animation_state : std::uint8_t
        {
            hidden,
            opening,
            visible,
            closing
        };

        c_popup_menu();
        ~c_popup_menu() = default;

        void render() const;
        void set_projection(const glm::mat4 &proj);

        // Event handling
        auto on_mouse_move(glm::vec2 mouse_position) -> void;
        auto on_mouse_press(glm::vec2 mouse_position) -> bool; // Returns true if menu handled the click
        auto on_right_click(glm::vec2 mouse_position) -> void;

        // Menu management
        void add_menu_item(const std::string &text, std::function<void()> callback = nullptr);
        void clear_menu_items();
        void show_at_position(glm::vec2 position);
        void hide();
        [[nodiscard]] auto is_visible() const -> bool;

        // Animation update
        void update(float delta_time);

    private:
        glm::vec2 m_position;
        glm::vec2 m_target_position;
        glm::vec2 m_click_origin; // Where the right-click happened
        glm::mat4 m_projection;
        std::vector<s_menu_item> m_menu_items;

        // Animation properties
        e_animation_state m_animation_state = e_animation_state::hidden;
        float m_animation_time = 0.0F;
        float m_animation_duration = 0.3F; // 300ms for smooth sweep

        // Visual properties
        float m_menu_item_padding = 15.0F;
        float m_menu_item_height = 35.0F;
        float m_menu_width = 180.0F;
        float m_menu_margin = 8.0F;

        // Internal methods
        void update_menu_item_sizes();
        auto get_menu_item_at_position(glm::vec2 position) -> s_menu_item *;
        auto get_menu_bounds() const -> std::pair<glm::vec2, glm::vec2>; // Returns min/max bounds
        auto calculate_animation_progress() const -> float;
        auto get_current_size() const -> glm::vec2; // For sweep animation
        auto get_current_position() const -> glm::vec2;
        auto get_current_alpha() const -> float;
    };
} // namespace gui

// Implementation
namespace gui
{
    c_popup_menu::c_popup_menu()
        : m_position(0.0F),
          m_target_position(0.0F),
          m_click_origin(0.0F),
          m_projection(1.F)
    {
        auto init = [&]
        {
            add_menu_item("File", []() { /* TODO: Implement file menu */ });
            add_menu_item("Edit", []() { /* TODO: Implement edit menu */ });
            add_menu_item("View", []() { /* TODO: Implement view menu */ });
            add_menu_item("Effects", []() { /* TODO: Implement effects menu */ });
            add_menu_item("Help", []() { /* TODO: Implement help menu */ });

            update_menu_item_sizes();
        };
        utility::c_notifier::subscribe(init);
    }

    void c_popup_menu::render() const
    {
        if (m_animation_state == e_animation_state::hidden)
        {
            return;
        }

        auto current_pos = get_current_position();
        auto current_size = get_current_size();
        auto alpha = get_current_alpha();

        // Menu background with better colors inspired by panels
        glm::vec4 menu_bg_color = { 0.06F, 0.06F, 0.12F, 0.95F * alpha }; // Dark blue-ish like panels
        opengl::shapes::c_rectangle(current_pos, current_size, menu_bg_color).draw(opengl::c_renderer{}, m_projection);

        // Menu border with subtle glow effect
        glm::vec4 border_color = { 0.22F, 0.22F, 0.28F, 0.8F * alpha }; // Subtle purple-gray border
        float border_thickness = 1.5F;

        // Top border
        opengl::shapes::c_rectangle(current_pos + glm::vec2{ 0, current_size.y - border_thickness },
                                    { current_size.x, border_thickness }, border_color)
            .draw(opengl::c_renderer{}, m_projection);
        // Bottom border
        opengl::shapes::c_rectangle(current_pos, { current_size.x, border_thickness }, border_color)
            .draw(opengl::c_renderer{}, m_projection);
        // Left border
        opengl::shapes::c_rectangle(current_pos, { border_thickness, current_size.y }, border_color)
            .draw(opengl::c_renderer{}, m_projection);
        // Right border
        opengl::shapes::c_rectangle(current_pos + glm::vec2{ current_size.x - border_thickness, 0 },
                                    { border_thickness, current_size.y }, border_color)
            .draw(opengl::c_renderer{}, m_projection);

        // Only render menu items if the menu is big enough to show them
        float full_height = (static_cast<float>(m_menu_items.size()) * m_menu_item_height) + (2.0F * m_menu_margin);
        if (current_size.y < m_menu_item_height / 2.0F || current_size.x < m_menu_width / 3.0F)
        {
            return; // Too small to show items yet
        }

        // Menu items
        for (std::size_t i = 0; i < m_menu_items.size(); ++i)
        {
            const auto &item = m_menu_items[i];

            float item_y = current_pos.y + full_height - m_menu_margin - (static_cast<float>(i + 1) * m_menu_item_height);
            glm::vec2 item_pos = { current_pos.x, item_y };
            glm::vec2 item_size = { current_size.x, m_menu_item_height };

            // Skip items that are outside the current animated size
            if (item_y < current_pos.y || item_y + m_menu_item_height > current_pos.y + current_size.y)
            {
                continue;
            }

            // Hover background with panel-inspired color
            if (item.is_hovered)
            {
                glm::vec4 hover_color = { 0.15F, 0.15F, 0.25F, 0.9F * alpha }; // Slightly brighter blue-purple
                opengl::shapes::c_rectangle(item_pos, item_size, hover_color).draw(opengl::c_renderer{}, m_projection);
            }

            // Text with better contrast
            glm::vec3 menu_text_color = item.is_hovered ? glm::vec3{ 1.0F, 1.0F, 1.0F } * alpha : // Pure white when hovered
                                            glm::vec3{ 0.9F, 0.9F, 0.95F } * alpha;               // Light blue-white for normal text

            auto text_size = opengl::c_text_renderer::instance().get_size(item.text, 1.0F);
            glm::vec2 text_pos = {
                item_pos.x + m_menu_item_padding,
                item_pos.y + ((m_menu_item_height - text_size.y) / 2.0F)
            };

            opengl::c_text_renderer::instance().submit(item.text, text_pos, 1.0F, menu_text_color);
        }
    }

    void c_popup_menu::set_projection(const glm::mat4 &proj)
    {
        m_projection = proj;
    }

    auto c_popup_menu::on_mouse_move(glm::vec2 mouse_position) -> void
    {
        if (!is_visible())
        {
            return;
        }

        // Reset all hover states
        for (auto &item : m_menu_items)
        {
            item.is_hovered = false;
        }

        // Check if mouse is over a menu item
        auto *hovered_item = get_menu_item_at_position(mouse_position);
        if (hovered_item != nullptr)
        {
            hovered_item->is_hovered = true;
        }
    }

    auto c_popup_menu::on_mouse_press(glm::vec2 mouse_position) -> bool
    {
        if (!is_visible())
        {
            return false;
        }

        auto *clicked_item = get_menu_item_at_position(mouse_position);
        if (clicked_item != nullptr && clicked_item->callback)
        {
            clicked_item->callback();
            hide(); // Hide menu after selection
            return true;
        }

        // Check if click is outside menu bounds
        auto [min_bounds, max_bounds] = get_menu_bounds();
        if (mouse_position.x < min_bounds.x || mouse_position.x > max_bounds.x || mouse_position.y < min_bounds.y || mouse_position.y > max_bounds.y)
        {
            hide(); // Hide menu if clicked outside
            return false;
        }

        return true; // Click was inside menu but not on item
    }

    auto c_popup_menu::on_right_click(glm::vec2 mouse_position) -> void
    {
        show_at_position(mouse_position);
    }

    void c_popup_menu::add_menu_item(const std::string &text, std::function<void()> callback)
    {
        s_menu_item item;
        item.text = text;
        item.callback = std::move(callback);
        item.width = opengl::c_text_renderer::instance().get_size(text, 1.0F).x;
        item.height = m_menu_item_height;
        m_menu_items.push_back(item);
        update_menu_item_sizes();
    }

    void c_popup_menu::clear_menu_items()
    {
        m_menu_items.clear();
    }

    void c_popup_menu::show_at_position(glm::vec2 position)
    {
        m_click_origin = position;
        m_target_position = position;
        m_animation_state = e_animation_state::opening;
        m_animation_time = 0.0F;
    }

    void c_popup_menu::hide()
    {
        if (m_animation_state == e_animation_state::visible || m_animation_state == e_animation_state::opening)
        {
            m_animation_state = e_animation_state::closing;
            m_animation_time = 0.0F;
        }
    }

    auto c_popup_menu::is_visible() const -> bool
    {
        return m_animation_state == e_animation_state::visible || m_animation_state == e_animation_state::opening || m_animation_state == e_animation_state::closing;
    }

    void c_popup_menu::update(float delta_time)
    {
        if (m_animation_state == e_animation_state::hidden)
        {
            return;
        }

        m_animation_time += delta_time;

        if (m_animation_state == e_animation_state::opening)
        {
            if (m_animation_time >= m_animation_duration)
            {
                m_animation_state = e_animation_state::visible;
                m_animation_time = m_animation_duration;
            }
        }
        else if (m_animation_state == e_animation_state::closing)
        {
            if (m_animation_time >= m_animation_duration)
            {
                m_animation_state = e_animation_state::hidden;
                m_animation_time = 0.0F;
            }
        }
    }

    void c_popup_menu::update_menu_item_sizes()
    {
        for (auto &item : m_menu_items)
        {
            item.width = opengl::c_text_renderer::instance().get_size(item.text, 1.0F).x;
            item.height = m_menu_item_height;
        }
    }

    auto c_popup_menu::get_menu_item_at_position(glm::vec2 position) -> s_menu_item *
    {
        auto [min_bounds, max_bounds] = get_menu_bounds();

        // Check if position is within menu bounds first
        if (position.x < min_bounds.x || position.x > max_bounds.x || position.y < min_bounds.y || position.y > max_bounds.y)
        {
            return nullptr;
        }

        auto current_pos = get_current_position();
        float menu_height = static_cast<float>(m_menu_items.size()) * m_menu_item_height + 2.0F * m_menu_margin;

        for (std::size_t i = 0; i < m_menu_items.size(); ++i)
        {
            float item_y = current_pos.y + menu_height - m_menu_margin - (static_cast<float>(i + 1) * m_menu_item_height);

            if (position.y >= item_y && position.y <= item_y + m_menu_item_height)
            {
                return &m_menu_items[i];
            }
        }

        return nullptr;
    }

    auto c_popup_menu::get_menu_bounds() const -> std::pair<glm::vec2, glm::vec2>
    {
        auto current_pos = get_current_position();
        float menu_height = static_cast<float>(m_menu_items.size()) * m_menu_item_height + 2.0F * m_menu_margin;

        glm::vec2 min_bounds = current_pos;
        glm::vec2 max_bounds = current_pos + glm::vec2{ m_menu_width, menu_height };

        return { min_bounds, max_bounds };
    }

    auto c_popup_menu::calculate_animation_progress() const -> float
    {
        if (m_animation_duration <= 0.0F)
        {
            return 1.0F;
        }
        return std::min(m_animation_time / m_animation_duration, 1.0F);
    }

    auto c_popup_menu::get_current_size() const -> glm::vec2
    {
        float full_width = m_menu_width;
        float full_height = (static_cast<float>(m_menu_items.size()) * m_menu_item_height) + (2.0F * m_menu_margin);

        if (m_animation_state == e_animation_state::visible)
        {
            return { full_width, full_height };
        }

        auto progress = calculate_animation_progress();

        if (m_animation_state == e_animation_state::opening)
        {
            // Sweep from click point (top-left) to full size (bottom-right)
            return { full_width * progress, full_height * progress };
        }

        if (m_animation_state == e_animation_state::closing)
        {
            // Sweep from full size to click point (bottom-right to top-left)
            auto reverse_progress = 1.0F - progress;
            return { full_width * reverse_progress, full_height * reverse_progress };
        }

        return { full_width, full_height };
    }

    auto c_popup_menu::get_current_position() const -> glm::vec2
    {
        if (m_animation_state == e_animation_state::visible)
        {
            return m_target_position;
        }

        // For sweep animation, position stays at click origin during opening
        // and animates towards click origin during closing
        if (m_animation_state == e_animation_state::opening)
        {
            return m_click_origin; // Menu grows from click point
        }

        if (m_animation_state == e_animation_state::closing)
        {
            auto current_size = get_current_size();

            // Calculate where the bottom-right should be when shrinking
            auto full_size = glm::vec2{ m_menu_width, (static_cast<float>(m_menu_items.size()) * m_menu_item_height) + (2.0F * m_menu_margin) };
            auto shrink_offset = full_size - current_size;

            // Move position so bottom-right moves towards click origin
            return m_target_position + shrink_offset;
        }

        return m_target_position;
    }
    auto c_popup_menu::get_current_alpha() const -> float
    {
        if (m_animation_state == e_animation_state::visible)
        {
            return 1.0F;
        }

        auto progress = calculate_animation_progress();

        if (m_animation_state == e_animation_state::opening)
        {
            return progress;
        }

        if (m_animation_state == e_animation_state::closing)
        {
            return 1.0F - progress;
        }

        return 0.0F;
    }
} // namespace gui
