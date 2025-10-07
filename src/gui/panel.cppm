module;
#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>
export module gui:panel;

import opengl;
import glm;
import utility;

namespace
{
    constexpr float resize_margin = 5.0F; // pixels

    enum class e_panel_state : std::uint8_t
    {
        normal,
        minimized,
    };

    enum class e_button_type : std::uint8_t
    {
        none = 0,
        minimize,
        close
    };

    enum class e_resize_direction : std::uint8_t
    {
        left = 0,
        right,
        top,
        bottom,
        top_left,
        top_right,
        bottom_left,
        bottom_right
    };

    enum class e_drag_state : std::uint8_t
    {
        none = 0,
        dragging,
        resizing
    };

    struct s_resize_handle
    {
        glm::vec2 original_mouse_pos{};
        glm::vec2 original_panel_size{};
        glm::vec2 original_panel_pos{};
        e_resize_direction direction;
    };

    struct s_button
    {
        glm::vec2 position{};
        glm::vec2 size{};
        e_button_type type{ e_button_type::none };
        bool hovered = false;
        bool pressed = false;
        std::function<void()> callback;
    };

} // namespace

export namespace gui
{
    enum class e_mouse_button : std::uint8_t
    {
        left = 0,
        right,
        middle
    };

    /**
     * A generalized GUI panel class with basic window functionalities like moving, resizing, minimizing, and closing.
     *
     * The Panel class provides a flexible framework for creating interactive panels in a GUI application. It supports essential features such as:
     *   - Moving: Click and drag the title bar to reposition the panel.
     *   - Resizing: Drag the edges or corners to adjust the panel size (if enabled).
     *   - Minimizing: Collapse the panel to just the title bar.
     *   - Closing: Remove the panel from view.
     *   - Customizable title bar with buttons for minimize and close actions.
     *   - Event handling for mouse interactions.
     *
     * The class is designed to be extended, allowing developers to override methods for custom rendering and behavior.
     */
    class c_panel
    {
    public:
        c_panel(glm::vec2 position, glm::vec2 size, std::string title = "Panel");
        virtual ~c_panel() = default;

        // Core functionality
        auto update_location(glm::vec2 new_location) -> void;
        auto update_size(glm::vec2 new_size) -> void;
        auto set_title(const std::string &title) -> void;

        // State management
        auto minimize() -> void;
        auto restore() -> void;
        auto close() -> void;

        // Event handling
        virtual auto on_mouse_move(glm::vec2 mouse_position) -> void;
        virtual auto on_mouse_press(glm::vec2 mouse_position, e_mouse_button button) -> void;
        virtual auto on_mouse_release(glm::vec2 mouse_position, e_mouse_button button) -> void;
        virtual auto on_resize(float width, float height) -> void;

        // Rendering
        virtual auto render() const -> void;

        // Accessors
        auto is_minimized() const -> bool;
        auto is_closed() const -> bool;

        auto get_location() const -> glm::vec2;
        auto get_size() const -> glm::vec2;
        auto get_title() const -> std::string;

        // Configuration
        auto set_resizable(bool resizable) -> void;
        auto set_movable(bool movable) -> void;
        auto set_closable(bool closable) -> void;

        // Set projection matrix for proper 2D rendering
        auto set_projection_matrix(const glm::mat4 &projection) -> void;

    protected:
        virtual auto render_content() const -> void {};
        virtual auto on_panel_closed() -> void {};
        virtual auto on_panel_minimized() -> void {};
        virtual auto on_panel_restored() -> void {};

        auto renderer() const -> opengl::c_renderer &;

    private:
        // Core properties
        glm::vec2 m_location{};
        glm::vec2 m_size{};
        glm::vec2 m_original_size{};
        glm::vec2 m_original_location{};
        glm::mat4 m_projection{};
        std::string m_title;

        // State
        e_panel_state m_state = e_panel_state::normal;
        bool m_closed = false;
        e_drag_state m_drag_state = e_drag_state::none;
        s_resize_handle m_resize_handle{};
        std::array<bool, 3> m_mouse_buttons{};

        // Configuration flags
        bool m_resizable = true;
        bool m_movable = true;
        bool m_closable = true;

        // UI elements
        std::vector<s_button> m_buttons;
        float m_title_bar_height = 30.0F;

        // Mouse state
        glm::vec2 m_last_mouse_pos{};
        glm::vec2 m_drag_offset{};

        // Rendering
        mutable opengl::c_renderer m_renderer;
        opengl::c_shader m_shader{ SOURCE_DIR "/src/shaders/panel_shader.glsl" };

        // Helper methods
        auto init_buttons() -> void;
        auto update_button_positions() -> void;
        auto is_point_in_title_bar(glm::vec2 point) const -> bool;
        auto is_point_in_panel(glm::vec2 point) const -> bool;
        auto get_resize_direction(glm::vec2 point) const -> std::optional<e_resize_direction>;
        auto get_button_at_position(glm::vec2 position) -> s_button *;
        auto drag_or_resize_panel(glm::vec2 mouse_position) -> void;
        auto render_title_bar() const -> void;
        auto render_buttons() const -> void;
        static auto get_button_color(const s_button &button) -> glm::vec4;
    };
} // namespace gui

// Implementation
namespace gui
{
    c_panel::c_panel(glm::vec2 position, glm::vec2 size, std::string title)
        : m_location{ position },
          m_size{ size },
          m_original_size{ size },
          m_original_location{ position },
          m_projection{ 1.F },
          m_title{ std::move(title) }
    {
        init_buttons();
        update_button_positions();
        auto init = [this]
        {
            m_shader.set_uniform_4f("u_fill_color", { 0.2F, 0.2F, 0.2F, 0.9F });
            m_shader.set_uniform_4f("u_border_color", { 0.17F, 0.17F, 0.17F, 0.5F });
            m_shader.set_uniform_1f("u_border_radius", 10);
            m_shader.set_uniform_1f("u_border_thickness", 10);
        };
        utility::c_notifier::subscribe(init);
    }

    auto c_panel::update_location(glm::vec2 new_location) -> void
    {
        m_location = new_location;
        m_shader.set_uniform_2f("u_panel_pos", m_location);
        update_button_positions();
    }

    auto c_panel::update_size(glm::vec2 new_size) -> void
    {
        m_size = new_size;
        m_shader.set_uniform_2f("u_panel_size", m_size);
        update_button_positions();
    }

    auto c_panel::set_title(const std::string &title) -> void
    {
        m_title = title;
    }

    auto c_panel::minimize() -> void
    {
        if (m_state != e_panel_state::minimized)
        {
            m_original_size = m_size;
            m_original_location = m_location;
            m_state = e_panel_state::minimized;
            m_size.y = m_title_bar_height;
            on_panel_minimized();
            update_button_positions();
        }
    }

    auto c_panel::restore() -> void
    {
        if (m_state != e_panel_state::normal)
        {
            m_state = e_panel_state::normal;
            m_location = m_original_location;
            m_size = m_original_size;
            on_panel_restored();
            update_button_positions();
        }
    }

    auto c_panel::close() -> void
    {
        m_closed = true;
        on_panel_closed();
    }

    auto c_panel::on_mouse_move(glm::vec2 mouse_position) -> void
    {
        // Update button hover states
        for (auto &button : m_buttons)
        {
            button.hovered = (mouse_position.x >= button.position.x and mouse_position.x <= button.position.x + button.size.x and mouse_position.y >= button.position.y and mouse_position.y <= button.position.y + button.size.y);
        }

        drag_or_resize_panel(mouse_position);

        m_last_mouse_pos = mouse_position;
    }

    auto c_panel::on_mouse_press(glm::vec2 mouse_position, e_mouse_button button) -> void
    {
        using enum e_mouse_button;
        using enum e_drag_state;

        m_last_mouse_pos = mouse_position;
        m_mouse_buttons[static_cast<std::size_t>(button)] = true;

        // Check button clicks
        if (button == left)
        {
            s_button *clicked_button = get_button_at_position(mouse_position);
            if (clicked_button)
            {
                clicked_button->pressed = true;
                if (clicked_button->callback)
                {
                    clicked_button->callback();
                }
                return;
            }

            if (auto direction = get_resize_direction(mouse_position); m_resizable and direction)
            {
                m_drag_state = resizing;
                m_resize_handle.direction = *direction;
                m_resize_handle.original_mouse_pos = mouse_position;
                m_resize_handle.original_panel_size = m_size;
                m_resize_handle.original_panel_pos = m_location;
            }
            else if (is_point_in_title_bar(mouse_position) and m_movable)
            {
                m_drag_state = dragging;
                m_drag_offset = mouse_position - m_location;
            }
        }
    }

    auto c_panel::on_mouse_release(glm::vec2 /*position*/, e_mouse_button button) -> void
    {
        using enum e_mouse_button;
        m_drag_state = e_drag_state::none;
        m_mouse_buttons[static_cast<std::size_t>(button)] = false;

        // Reset button pressed states
        for (auto &panel_button : m_buttons)
        {
            panel_button.pressed = false;
        }
    }

    auto c_panel::on_resize(float width, float height) -> void
    {
        update_size({ width, height });
        update_button_positions();
    }

    auto c_panel::render() const -> void
    {
        if (m_closed)
        {
            return;
        }

        // Draw panel background
        std::vector<glm::vec2> panel_corners = {
            m_location,
            { m_location.x + m_size.x, m_location.y },
            { m_location.x + m_size.x, m_location.y + m_size.y },
            { m_location.x, m_location.y + m_size.y }
        };
        opengl::c_buffer_layout layout;
        layout.push<float>(2); // Position
        opengl::c_vertex_buffer<glm::vec2> vertex_buffer(panel_corners, panel_corners.size());
        opengl::c_index_buffer index_buffer({ 0, 1, 2, 2, 3, 0 }, 6);
        opengl::c_vertex_array vertex_array;
        vertex_array.add_buffer(vertex_buffer, layout);
        opengl::c_renderer renderer;
        renderer.draw(vertex_array, vertex_buffer, index_buffer, m_shader);

        // Render title bar
        render_title_bar();

        // Render buttons
        render_buttons();

        // Render content area (only if not minimized)
        if (!is_minimized())
        {
            render_content();
        }
    }

    auto c_panel::init_buttons() -> void
    {
        m_buttons.clear();

        if (m_closable)
        {
            s_button close_btn;
            close_btn.type = e_button_type::close;
            close_btn.size = { 20.0F, 20.0F };
            close_btn.callback = [this]()
            {
                close();
            };
            m_buttons.push_back(close_btn);
        }

        s_button minimize_btn;
        minimize_btn.type = e_button_type::minimize;
        minimize_btn.size = { 20.0F, 20.0F };
        minimize_btn.callback = [this]()
        {
            if (is_minimized())
            {
                restore();
            }
            else
            {
                minimize();
            }
        };
        m_buttons.push_back(minimize_btn);
    }

    auto c_panel::update_button_positions() -> void
    {
        float button_spacing = 5.0F;
        float right_margin = 10.0F;
        float current_x = m_location.x + m_size.x - right_margin;

        for (auto &button : m_buttons)
        {
            current_x -= button.size.x;
            button.position.x = current_x;
            button.position.y = m_location.y + m_size.y - m_title_bar_height + (m_title_bar_height - button.size.y) * 0.5F;
            current_x -= button_spacing;
        }
    }

    auto c_panel::is_point_in_title_bar(glm::vec2 point) const -> bool
    {
        return (point.x >= m_location.x and point.x <= m_location.x + m_size.x and point.y >= m_location.y + m_size.y - m_title_bar_height and point.y <= m_location.y + m_size.y);
    }

    auto c_panel::is_point_in_panel(glm::vec2 point) const -> bool
    {
        return (point.x >= m_location.x and point.x <= m_location.x + m_size.x and point.y >= m_location.y and point.y <= m_location.y + m_size.y);
    }

    auto c_panel::get_resize_direction(glm::vec2 point) const -> std::optional<e_resize_direction>
    {
        const float x_min = m_location.x;
        const float x_max = m_location.x + m_size.x;
        const float y_min = m_location.y;
        const float y_max = m_location.y + m_size.y;

        bool near_left = std::abs(point.x - x_min) <= resize_margin;
        bool near_right = std::abs(point.x - x_max) <= resize_margin;
        bool near_bottom = std::abs(point.y - y_min) <= resize_margin;
        bool near_top = std::abs(point.y - y_max) <= resize_margin;

        // Inside panel bounds?
        bool inside_x = point.x >= x_min and point.x <= x_max;
        bool inside_y = point.y >= y_min and point.y <= y_max;

        // Corners first (priority)
        if (near_left and near_top)
        {
            return e_resize_direction::top_left;
        }
        if (near_right and near_top)
        {
            return e_resize_direction::top_right;
        }
        if (near_bottom and near_left)
        {
            return e_resize_direction::bottom_left;
        }
        if (near_right and near_bottom)
        {
            return e_resize_direction::bottom_right;
        }

        // Edges next
        if (near_left and inside_y)
        {
            return e_resize_direction::left;
        }
        if (near_right and inside_y)
        {
            return e_resize_direction::right;
        }
        if (near_top and inside_x)
        {
            return e_resize_direction::top;
        }
        if (near_bottom and inside_x)
        {
            return e_resize_direction::bottom;
        }

        // Not near any edge
        return std::nullopt;
    }

    auto c_panel::get_button_at_position(glm::vec2 mouse_position) -> s_button *
    {
        for (auto &button : m_buttons)
        {
            if (mouse_position.x >= button.position.x and mouse_position.x <= button.position.x + button.size.x and mouse_position.y >= button.position.y and mouse_position.y <= button.position.y + button.size.y)
            {
                return &button;
            }
        }
        return nullptr;
    }

    auto c_panel::render_title_bar() const -> void
    {
        glm::vec2 title_bar_pos = m_location + glm::vec2{ 0, m_size.y - m_title_bar_height };
        glm::vec2 title_bar_size = { m_size.x, m_title_bar_height };
        glm::vec4 title_bar_color = { 0.3F, 0.3F, 0.3F, 1.0F };

        opengl::shapes::c_rectangle(title_bar_pos, title_bar_size, title_bar_color).draw(m_renderer, m_projection);
        auto title_text_size = opengl::c_text_renderer::instance().get_size(m_title, 1.F);
        opengl::c_text_renderer::instance().submit(m_title, title_bar_pos + glm::vec2{ (title_bar_size.x - title_text_size.x) / 2, (title_text_size.y) / 2 }, 1.F, glm::vec3{ 0.F, 1.F, 1.F });
    }

    auto c_panel::render_buttons() const -> void
    {
        for (const auto &button : m_buttons)
        {
            glm::vec4 button_color = get_button_color(button);
            opengl::shapes::c_rectangle(button.position, button.size, button_color).draw(m_renderer, m_projection);

            // Draw button symbols
            glm::vec2 button_center = button.position + button.size * 0.5F;
            float symbol_size = button.size.x * 0.3F;
            glm::vec4 symbol_color = { 1.0F, 1.0F, 1.0F, 1.0F }; // White

            if (button.type == e_button_type::close)
            {
                // Draw an 'X' using two lines
                glm::vec2 line1_start = button_center + glm::vec2(-symbol_size, -symbol_size);
                glm::vec2 line1_end = button_center + glm::vec2(symbol_size, symbol_size);
                glm::vec2 line2_start = button_center + glm::vec2(-symbol_size, symbol_size);
                glm::vec2 line2_end = button_center + glm::vec2(symbol_size, -symbol_size);

                opengl::shapes::c_line(line1_start, line1_end, symbol_color).draw(m_renderer, m_projection);
                opengl::shapes::c_line(line2_start, line2_end, symbol_color).draw(m_renderer, m_projection);
            }
            else if (button.type == e_button_type::minimize)
            {
                // Draw a horizontal line
                glm::vec2 line_start = button_center + glm::vec2(-symbol_size, 0.0F);
                glm::vec2 line_end = button_center + glm::vec2(symbol_size, 0.0F);
                opengl::shapes::c_line(line_start, line_end, symbol_color).draw(m_renderer, m_projection);
            }
        }
    }

    auto c_panel::get_button_color(const s_button &button) -> glm::vec4
    {
        if (button.pressed)
        {
            switch (button.type)
            {
            case e_button_type::close:
                return { 0.8F, 0.2F, 0.2F, 1.0F };
            case e_button_type::minimize:
                return { 0.6F, 0.6F, 0.2F, 1.0F };
            default:
                break;
            }
        }
        else if (button.hovered)
        {
            switch (button.type)
            {
            case e_button_type::close:
                return { 0.9F, 0.3F, 0.3F, 1.0F };
            case e_button_type::minimize:
                return { 0.7F, 0.7F, 0.3F, 1.0F };
            default:
                break;
            }
        }

        // Default button colors
        switch (button.type)
        {
        case e_button_type::close:
            return { 0.7F, 0.2F, 0.2F, 1.0F };
        case e_button_type::minimize:
            return { 0.5F, 0.5F, 0.2F, 1.0F };
        default:
            break;
        }

        return { 0.5F, 0.5F, 0.5F, 1.0F };
    }

    auto c_panel::set_closable(bool closable) -> void
    {
        m_closable = closable;
    }

    auto c_panel::set_movable(bool movable) -> void
    {
        m_movable = movable;
    }

    auto c_panel::set_resizable(bool resizable) -> void
    {
        m_resizable = resizable;
    }

    auto c_panel::get_title() const -> std::string
    {
        return m_title;
    }

    auto c_panel::get_size() const -> glm::vec2
    {
        return m_size;
    }

    auto c_panel::get_location() const -> glm::vec2
    {
        return m_location;
    }

    auto c_panel::drag_or_resize_panel(glm::vec2 mouse_position) -> void
    {
        using enum e_drag_state;
        using enum e_resize_direction;

        if (m_drag_state == dragging and m_movable)
        {
            m_location = mouse_position - m_drag_offset;
            update_button_positions();
        }
        else if (m_drag_state == resizing)
        {
            glm::vec2 delta = mouse_position - m_resize_handle.original_mouse_pos;

            switch (m_resize_handle.direction)
            {
            case left:
                m_location.x = m_resize_handle.original_panel_pos.x + delta.x;
                m_size.x = m_resize_handle.original_panel_size.x - delta.x;
                break;
            case right:
                m_size.x = m_resize_handle.original_panel_size.x + delta.x;
                break;
            case top:
                m_size.y = m_resize_handle.original_panel_size.y + delta.y;
                break;
            case bottom:
                m_location.y = m_resize_handle.original_panel_pos.y + delta.y;
                m_size.y = m_resize_handle.original_panel_size.y - delta.y;
                break;
            case top_left:
                m_location.x = m_resize_handle.original_panel_pos.x + delta.x;
                m_size.x = m_resize_handle.original_panel_size.x - delta.x;
                m_size.y = m_resize_handle.original_panel_size.y + delta.y;
                break;
            case top_right:
                m_size.x = m_resize_handle.original_panel_size.x + delta.x;
                m_size.y = m_resize_handle.original_panel_size.y + delta.y;
                break;
            case bottom_left:
                m_location.x = m_resize_handle.original_panel_pos.x + delta.x;
                m_size.x = m_resize_handle.original_panel_size.x - delta.x;
                m_location.y = m_resize_handle.original_panel_pos.y + delta.y;
                m_size.y = m_resize_handle.original_panel_size.y - delta.y;
                break;
            case bottom_right:
                m_size.x = m_resize_handle.original_panel_size.x + delta.x;
                m_location.y = m_resize_handle.original_panel_pos.y + delta.y;
                m_size.y = m_resize_handle.original_panel_size.y - delta.y;
                break;
            default:
                break;
            }
        }

        m_shader.set_uniform_2f("u_panel_pos", m_location);
        m_shader.set_uniform_2f("u_panel_size", m_size);
        update_button_positions();
    }

    auto c_panel::is_closed() const -> bool
    {
        return m_closed;
    }

    auto c_panel::is_minimized() const -> bool
    {
        return m_state == e_panel_state::minimized;
    }

    auto c_panel::set_projection_matrix(const glm::mat4 &projection) -> void
    {
        m_projection = projection;
        m_shader.set_uniform_mat4f("projection", m_projection);
    }

    auto c_panel::renderer() const -> opengl::c_renderer &
    {
        return m_renderer;
    }
} // namespace gui
