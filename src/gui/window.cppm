module;
#include <GL/glew.h>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <print>
#include <ranges>
#include <vector>

export module gui:window;

import music;
import math;
import opengl;
import glm;

export namespace gui
{
    class c_window
    {
    public:
        c_window(int width, int height, const char *title);

        void show();

    private:
        std::unique_ptr<sf::Window> m_window;
        std::unique_ptr<music::c_track> m_track;

        float m_max_intensity{};
        std::vector<opengl::s_vertex> m_vertices;
        std::vector<unsigned int> m_indices;
        opengl::c_mesh m_mesh;
        opengl::c_renderer m_renderer;
        std::unique_ptr<opengl::c_shader> m_shader;

        void process_events();
        void render();
    };
} // namespace gui

// Implementation
namespace gui
{
    c_window::c_window(int width, int height, const char *title)
        : m_mesh(m_vertices, m_indices)
    {
        sf::ContextSettings settings;
        settings.majorVersion = 4;
        settings.minorVersion = 5;
        m_window = std::make_unique<sf::Window>(sf::VideoMode(width, height), title, sf::Style::Default, settings);
        m_window->setVerticalSyncEnabled(true);

        if (glewInit() != GLEW_OK)
        {
            std::cerr << "Error initializing GLEW" << '\n';
        }

        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(opengl::gl_debug_callback_fn, nullptr);

        m_mesh.setup_mesh();
        m_shader = std::make_unique<opengl::c_shader>("../src/shaders/basic.shader");

        std::print("Enter the path to the music file: ");
        std::string path;
        std::cin >> path;
        m_track = std::make_unique<music::c_track>(path);
        m_track->play();
    }

    void c_window::show()
    {
        while (m_window->isOpen())
        {
            process_events();
            render();
        }
    }

    void c_window::process_events()
    {
        sf::Event event{};
        while (m_window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                m_window->close();
            }
            if (event.type == sf::Event::Resized)
            {
                glViewport(0, 0, event.size.width, event.size.height);
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Q)
            {
                m_window->close();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
            {
                if (m_track->is_playing())
                {
                    m_track->pause();
                }
                else
                {
                    m_track->play();
                }
            }
        }
    }

    void c_window::render()
    {
        auto [window_width, window_height] = m_window->getSize();
        auto data = m_track->get_samples();
        auto fft_data = math::fft(data)
                        | std::views::transform([](auto &&datum)
                                                { return std::abs(datum.real()); })
                        | std::ranges::to<std::vector>();
        m_max_intensity = std::max(*std::ranges::max_element(fft_data), m_max_intensity);
        auto base_freq = 10.F;
        float freq = base_freq;
        std::size_t count = 12 * (std::log2(fft_data.size()) - std::log2(base_freq));
        m_vertices.resize(fft_data.size() * 4); // A rectangle bar = 4 vertices
        m_indices.resize(fft_data.size() * 6);  // 6 indices per rectangle (2 triangles, i.e. vertex (1,2,3) and (2,4,3))

        for (std::size_t index{}; freq < fft_data.size(); ++index, freq *= std::pow(2, 1.F / 12.F))
        {
            auto value = fft_data[std::floor(freq)];
            float x_base = window_width / count * index;
            float y_base = window_height / m_max_intensity * value;
            m_vertices[(std::size_t)(index * 4) + 0] = opengl::s_vertex{ glm::vec3{ x_base, 0, 0 } };
            m_vertices[(std::size_t)(index * 4) + 1] = opengl::s_vertex{ glm::vec3{ (x_base + (window_width / count)), 0, 0 } };
            m_vertices[(std::size_t)(index * 4) + 2] = opengl::s_vertex{ glm::vec3{ x_base, y_base, 0 } };
            m_vertices[(std::size_t)(index * 4) + 3] = opengl::s_vertex{ glm::vec3{ (x_base + (window_width / count)), y_base, 0 } };
            m_indices[(std::size_t)(index * 6) + 0] = (unsigned)index * 4 + 0;
            m_indices[(std::size_t)(index * 6) + 1] = (unsigned)index * 4 + 1;
            m_indices[(std::size_t)(index * 6) + 2] = (unsigned)index * 4 + 2;
            m_indices[(std::size_t)(index * 6) + 3] = (unsigned)index * 4 + 2;
            m_indices[(std::size_t)(index * 6) + 4] = (unsigned)index * 4 + 1;
            m_indices[(std::size_t)(index * 6) + 5] = (unsigned)index * 4 + 3;
        }
        auto projection = glm::gtc::ortho(0.F, static_cast<float>(window_width), 0.F, static_cast<float>(window_height), -1.F, 1.F);
        m_shader->set_uniform_mat4f("projection", projection);
        m_shader->set_uniform_1f("u_screenWidth", static_cast<float>(window_width));
        m_mesh.update_mesh(m_vertices, m_indices);
        m_mesh.draw(m_renderer, *m_shader);
        m_window->display();
    }
} // namespace gui
