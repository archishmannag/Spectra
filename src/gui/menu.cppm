module;
#include <algorithm>
#include <functional>
#include <string>
#include <vector>
export module gui:menu;

import opengl;
import glm;
import utility;

export namespace gui
{
    class c_menu_bar
    {
    public:
        struct s_menu_item
        {
            std::string text;
            std::function<void()> callback;
            bool is_hovered = false;
            float x_position = 0.0F;
            float width = 0.0F;
        };

        c_menu_bar(glm::vec2 position, glm::vec2 size, const std::string &title);
        ~c_menu_bar() = default;

        void render(const glm::mat4 &projection) const;
        void set_projection(const glm::mat4 &proj);
        void update_size(glm::vec2 new_size);
        void update_position(glm::vec2 new_position);

        // Event handling
        auto on_mouse_move(glm::vec2 mouse_position) -> void;
        auto on_mouse_press(glm::vec2 mouse_position) -> void;

        // Menu management
        void add_menu_item(const std::string &text, std::function<void()> callback = nullptr);
        void clear_menu_items();

    private:
        glm::vec2 m_position;
        glm::vec2 m_size;
        std::string m_title;
        glm::mat4 m_projection;
        std::vector<s_menu_item> m_menu_items;

        // Visual properties
        float m_menu_bar_height = 40.0F;
        float m_menu_item_padding = 20.0F;
        float m_menu_item_spacing = 10.0F;
        float m_title_offset = 20.0F;

        // Internal methods
        void update_menu_item_positions();
        auto get_menu_item_at_position(glm::vec2 position) -> s_menu_item *;
    };
} // namespace gui

// Implementation
namespace gui
{
    c_menu_bar::c_menu_bar(glm::vec2 position, glm::vec2 size, const std::string &title)
        : m_position(position), m_size(size), m_title(title), m_projection(glm::mat4(1.0F))
    {
        auto init = [&]
        {
            add_menu_item("File", []() { /* TODO: Implement file menu */ });
            add_menu_item("Edit", []() { /* TODO: Implement edit menu */ });
            add_menu_item("View", []() { /* TODO: Implement view menu */ });
            add_menu_item("Effects", []() { /* TODO: Implement effects menu */ });
            add_menu_item("Help", []() { /* TODO: Implement help menu */ });

            update_menu_item_positions();
        };
        utility::c_notifier::subscribe(init);
    }
    void c_menu_bar::render(const glm::mat4 &projection) const
    {
        // Menu bar background
        glm::vec4 menu_bg_color = { 0.12F, 0.12F, 0.15F, 1.0F }; // Dark background like in reference
        opengl::shapes::c_rectangle(m_position, m_size, menu_bg_color).draw(opengl::c_renderer{}, projection);

        // Menu bar bottom border for definition
        glm::vec4 border_color = { 0.3F, 0.3F, 0.35F, 1.0F };
        float border_height = 1.0F;
        glm::vec2 border_pos = { m_position.x, m_position.y };
        glm::vec2 border_size = { m_size.x, border_height };
        opengl::shapes::c_rectangle(border_pos, border_size, border_color).draw(opengl::c_renderer{}, projection);

        // Application title (centered)
        auto title_text_size = opengl::c_text_renderer::instance().get_size(m_title, 1.2F);
        glm::vec2 title_pos = {
            m_position.x + (m_size.x - title_text_size.x) / 2.0F,
            m_position.y + (m_size.y - title_text_size.y) / 2.0F + 2.0F
        };
        glm::vec3 title_color = { 0.95F, 0.95F, 0.98F }; // Light color for title
        opengl::c_text_renderer::instance().submit(m_title, title_pos, 1.2F, title_color);

        // Menu items (left side)
        for (const auto &item : m_menu_items)
        {
            glm::vec3 menu_text_color = item.is_hovered ? glm::vec3{ 1.0F, 1.0F, 1.0F } : glm::vec3{ 0.8F, 0.8F, 0.85F };

            // Hover background
            if (item.is_hovered)
            {
                glm::vec4 hover_color = { 0.25F, 0.25F, 0.3F, 0.8F };
                glm::vec2 hover_pos = { item.x_position - m_menu_item_padding / 2.0F, m_position.y };
                glm::vec2 hover_size = { item.width + m_menu_item_padding, m_size.y };
                opengl::shapes::c_rectangle(hover_pos, hover_size, hover_color).draw(opengl::c_renderer{}, projection);
            }

            glm::vec2 text_pos = {
                item.x_position,
                m_position.y + (m_size.y - opengl::c_text_renderer::instance().get_size(item.text, 1.0F).y) / 2.0F + 2.0F
            };

            opengl::c_text_renderer::instance().submit(item.text, text_pos, 1.0F, menu_text_color);
        }
    }

    void c_menu_bar::set_projection(const glm::mat4 &proj)
    {
        m_projection = proj;
    }

    void c_menu_bar::update_size(glm::vec2 new_size)
    {
        m_size = new_size;
        update_menu_item_positions();
    }

    void c_menu_bar::update_position(glm::vec2 new_position)
    {
        m_position = new_position;
        update_menu_item_positions();
    }

    auto c_menu_bar::on_mouse_move(glm::vec2 mouse_position) -> void
    {
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

    auto c_menu_bar::on_mouse_press(glm::vec2 mouse_position) -> void
    {
        auto *clicked_item = get_menu_item_at_position(mouse_position);
        if (clicked_item != nullptr && clicked_item->callback)
        {
            clicked_item->callback();
        }
    }

    void c_menu_bar::add_menu_item(const std::string &text, std::function<void()> callback)
    {
        s_menu_item item;
        item.text = text;
        item.callback = std::move(callback);
        item.width = opengl::c_text_renderer::instance().get_size(text, 1.0F).x;
        m_menu_items.push_back(item);
        update_menu_item_positions();
    }

    void c_menu_bar::clear_menu_items()
    {
        m_menu_items.clear();
    }

    void c_menu_bar::update_menu_item_positions()
    {
        float current_x = m_position.x + m_title_offset;

        for (auto &item : m_menu_items)
        {
            item.x_position = current_x;
            current_x += item.width + m_menu_item_spacing + m_menu_item_padding;
        }
    }

    auto c_menu_bar::get_menu_item_at_position(glm::vec2 position) -> s_menu_item *
    {
        // Check if position is within menu bar bounds first
        if (position.x < m_position.x || position.x > m_position.x + m_size.x || position.y < m_position.y || position.y > m_position.y + m_size.y)
        {
            return nullptr;
        }

        for (auto &item : m_menu_items)
        {
            float item_left = item.x_position - m_menu_item_padding / 2.0F;
            float item_right = item.x_position + item.width + m_menu_item_padding / 2.0F;

            if (position.x >= item_left && position.x <= item_right)
            {
                return &item;
            }
        }

        return nullptr;
    }
} // namespace gui
