module;
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>
export module gui:window;

import :waveform;
import :tracks;
import :menu;

import utility;
import music;
import math;
import opengl;
import glm;

export namespace gui
{
    class c_window
    {
    public:
        c_window(int width, int height, const std::string &title);

        auto show() -> void;

    private:
        std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> m_window;
        std::vector<std::shared_ptr<music::c_track>> m_tracks;

        // Components
        c_popup_menu m_popup_menu;
        c_waveform_panel m_waveform_pane;
        c_track_panel m_track_panel;

        // Parent resources shared to components
        music::c_audio_manager m_audio_manager;

        auto register_event_callbacks() -> void;
        auto render() -> void;

        // Helper functions
        auto screen_to_opengl_coords(glm::vec2 screen_coords) const -> glm::vec2;
        auto set_projections() -> void;

        // Callbacks for GLFW events

        /**
         * @brief Key callback function to handle key events
         *
         * @param key The key that was pressed or released
         * @param scancode The scancode of the key
         * @param action The action that was performed (press, release, repeat)
         * @param mods The modifier keys that were active during the event
         * @return void
         *
         */
        auto key_callback(int key, int scancode, int action, int mods) -> void;
        auto framebuffer_size_callback(int width, int height) -> void;
        auto mouse_position_callback(double xpos, double ypos) -> void;
        auto mouse_button_callback(int button, int action, int mods) -> void;
        auto mouse_scroll_callback(double x_offset, double y_offset) -> void;
        auto path_drop_callback(int count, const char **paths) -> void;
    };
} // namespace gui

// Implementation
namespace gui
{
    c_window::c_window(int width, int height, const std::string &title)
        : m_window(nullptr, &glfwDestroyWindow),
          m_waveform_pane({ static_cast<float>(width) / 2.F, static_cast<float>(height) / 4.F },
                          { static_cast<float>(width) / 2.F, static_cast<float>(height) / 2.F }, m_audio_manager),
          m_track_panel({ 0.F, static_cast<float>(height) / 4.F },
                        { static_cast<float>(width) / 2.F, static_cast<float>(height) / 2.F }, m_tracks)
    {
        m_window = std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)>(glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr), &glfwDestroyWindow);
        if (not m_window)
        {
            throw std::runtime_error("Error creating SDL window.");
        }
        glfwMakeContextCurrent(m_window.get());
        glfwSetWindowUserPointer(m_window.get(), this);

        glewExperimental = GL_TRUE; // Enable modern OpenGL features
        auto err = glewInit();
        if (err != GLEW_OK)
        {
            std::cerr << "Error initializing GLEW" << '\n';
            std::cerr << "Error: " << glewGetErrorString(err) << '\n';
        }

        glfwSwapInterval(1); // Enable V-Sync
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(opengl::gl_debug_callback_fn, nullptr);

        // OpenGL initialized, now notify all subscribers to initialize themselves
        opengl::c_text_renderer::instance().load_font(SOURCE_DIR "/assets/fonts/NotoSans.ttf", 24);
        utility::c_notifier::notify();
    }

    auto c_window::screen_to_opengl_coords(glm::vec2 screen_coords) const -> glm::vec2
    {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(m_window.get(), &width, &height);
        // Convert screen coordinates (top-left origin) to OpenGL coordinates (bottom-left origin)
        return { screen_coords.x, static_cast<float>(height) - screen_coords.y };
    }

    auto c_window::set_projections() -> void
    {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(m_window.get(), &width, &height);
        glm::mat4 proj = glm::gtc::ortho(0.F, static_cast<float>(width), 0.F, static_cast<float>(height), -1.F, 1.F);
        m_waveform_pane.set_projection(proj);
        m_track_panel.set_projection(proj);
        m_popup_menu.set_projection(proj);
        opengl::c_text_renderer::instance().resize({ width, height });
    }

    auto c_window::show() -> void
    {
        register_event_callbacks();
        set_projections();
        while (not glfwWindowShouldClose(m_window.get()))
        {
            glfwPollEvents();
            double xpos = NAN;
            double ypos = NAN;
            glfwGetCursorPos(m_window.get(), &xpos, &ypos);
            auto opengl_coords = screen_to_opengl_coords({ xpos, ypos });

            // Update popup menu animation
            static auto last_time = std::chrono::steady_clock::now();
            auto current_time = std::chrono::steady_clock::now();
            auto delta_time = std::chrono::duration<float>(current_time - last_time).count();
            last_time = current_time;

            m_popup_menu.update(delta_time);
            m_popup_menu.on_mouse_move(opengl_coords);
            m_track_panel.set_mouse_position(opengl_coords);
            m_waveform_pane.set_mouse_position(opengl_coords);
            m_waveform_pane.update_waveform();
            render();
        }
    }

    auto c_window::key_callback(int key, int /*scancode*/, int action, int /*mods*/) -> void
    {
        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_SPACE)
            {
                if (m_audio_manager.is_playing())
                {
                    m_audio_manager.pause();
                }
                else
                {
                    m_audio_manager.play();
                }
            }
            if (key == GLFW_KEY_ESCAPE)
            {
                glfwSetWindowShouldClose(m_window.get(), 1);
            }
        }
    }

    auto c_window::framebuffer_size_callback(int width, int height) -> void
    {
        glm::mat4 proj = glm::gtc::ortho(0.F, static_cast<float>(width), 0.F, static_cast<float>(height), -1.F, 1.F);
        glViewport(0, 0, width, height);

        // Update popup menu projection
        m_popup_menu.set_projection(proj);

        // Update panels (use full height without menu bar)
        m_track_panel.set_projection(proj);
        m_track_panel.update_size({ static_cast<float>(width) / 2, static_cast<float>(height) / 2 });
        m_track_panel.update_location({ 0, static_cast<float>(height) / 4.F });
        m_waveform_pane.set_projection(proj);
        m_waveform_pane.update_size({ static_cast<float>(width) / 2, static_cast<float>(height) / 2 });
        m_waveform_pane.update_location({ static_cast<float>(width) / 2.F, static_cast<float>(height) / 4.F });
        opengl::c_text_renderer::instance().resize({ width, height });
    }

    auto c_window::mouse_position_callback(double xpos, double ypos) -> void
    {
        auto opengl_coords = screen_to_opengl_coords({ xpos, ypos });
        m_popup_menu.on_mouse_move(opengl_coords);
        m_track_panel.on_mouse_move(opengl_coords);
        m_waveform_pane.on_mouse_move(opengl_coords);
    }

    auto c_window::mouse_button_callback(int button, int action, int /*mods*/) -> void
    {
        using enum e_mouse_button;
        e_mouse_button e_button{};

        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            e_button = left;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            e_button = right;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            e_button = middle;
            break;
        default:
            return; // Unsupported button
        }
        double xpos = NAN;
        double ypos = NAN;
        glfwGetCursorPos(m_window.get(), &xpos, &ypos);

        if (action == GLFW_PRESS)
        {
            auto coords = screen_to_opengl_coords({ xpos, ypos });

            if (e_button == e_mouse_button::right)
            {
                // Right click - show popup menu
                m_popup_menu.on_right_click(coords);
            }
            else if (e_button == e_mouse_button::left)
            {
                // Left click - check if popup menu should handle it, otherwise pass to panels
                if (!m_popup_menu.on_mouse_press(coords))
                {
                    m_track_panel.on_mouse_press(coords, e_button);
                    m_waveform_pane.on_mouse_press(coords, e_button);
                }
            }
            else
            {
                m_track_panel.on_mouse_press(coords, e_button);
                m_waveform_pane.on_mouse_press(coords, e_button);
            }
        }
        else if (action == GLFW_RELEASE)
        {
            m_track_panel.on_mouse_release(screen_to_opengl_coords({ xpos, ypos }), e_button);
            m_waveform_pane.on_mouse_release(screen_to_opengl_coords({ xpos, ypos }), e_button);
        }
    }

    auto c_window::mouse_scroll_callback(double x_offset, double y_offset) -> void
    {
        double xpos = NAN;
        double ypos = NAN;
        glfwGetCursorPos(m_window.get(), &xpos, &ypos);
        m_track_panel.on_mouse_scroll({ xpos, ypos }, { x_offset, y_offset });
        m_waveform_pane.on_mouse_scroll({ xpos, ypos }, { x_offset, y_offset });
    }

    auto c_window::path_drop_callback(int count, const char **paths) -> void
    {
        static int curr_id = 0;
        if (count <= 0 or paths == nullptr)
        {
            return;
        }
        for (int i = 0; i < count; i++)
        {
            std::filesystem::path curr_path(paths[i]);
            if (not std::filesystem::exists(curr_path) or not std::filesystem::is_regular_file(curr_path))
            {
                continue;
            }
            try
            {
                auto track = std::make_shared<music::c_track>(curr_id++, curr_path);
                m_tracks.push_back(track);
                m_audio_manager.add_track(track);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error adding track: " << e.what() << '\n';
            }
        }
    }

    auto c_window::register_event_callbacks() -> void
    {
        auto key_callback = [](GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            static_cast<c_window *>(glfwGetWindowUserPointer(window))->key_callback(key, scancode, action, mods);
        };

        auto framebuffer_size_callback = [](GLFWwindow *window, int width, int height)
        {
            static_cast<c_window *>(glfwGetWindowUserPointer(window))->framebuffer_size_callback(width, height);
        };

        auto path_drop_callback = [](GLFWwindow *window, int count, const char **paths)
        {
            static_cast<c_window *>(glfwGetWindowUserPointer(window))->path_drop_callback(count, paths);
        };
        auto mouse_position_callback = [](GLFWwindow *window, double xpos, double ypos)
        {
            static_cast<c_window *>(glfwGetWindowUserPointer(window))->mouse_position_callback(xpos, ypos);
        };
        auto mouse_button_callback = [](GLFWwindow *window, int button, int action, int mods)
        {
            static_cast<c_window *>(glfwGetWindowUserPointer(window))->mouse_button_callback(button, action, mods);
        };
        auto mouse_scroll_callback = [](GLFWwindow *window, double x_offset, double y_offset)
        {
            static_cast<c_window *>(glfwGetWindowUserPointer(window))->mouse_scroll_callback(x_offset, y_offset);
        };

        glfwSetKeyCallback(m_window.get(), key_callback);
        glfwSetFramebufferSizeCallback(m_window.get(), framebuffer_size_callback);
        glfwSetDropCallback(m_window.get(), path_drop_callback);
        glfwSetCursorPosCallback(m_window.get(), mouse_position_callback);
        glfwSetMouseButtonCallback(m_window.get(), mouse_button_callback);
        glfwSetScrollCallback(m_window.get(), mouse_scroll_callback);
    }

    auto c_window::render() -> void
    {
        opengl::c_renderer::clear();

        m_waveform_pane.render();
        m_track_panel.render();
        m_popup_menu.render();
        opengl::c_text_renderer::instance().draw_texts();
        glfwSwapBuffers(m_window.get());
    }
} // namespace gui
