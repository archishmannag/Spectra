module;
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstring>
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
        c_window(int width, int height, const std::string &title);
        ~c_window();

        void show();

    private:
        std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> m_window;
        std::unique_ptr<opengl::c_shader> m_shader;

        music::c_audio_manager m_audio_manager;
        std::shared_ptr<music::c_track> m_track1;
        std::shared_ptr<music::c_track> m_track2;

        float m_max_intensity{};
        std::vector<float> m_audio_samples;
        std::vector<opengl::s_vertex> m_vertices;
        std::vector<unsigned int> m_indices;
        opengl::c_mesh m_mesh;
        opengl::c_renderer m_renderer;

        opengl::c_text_renderer m_text_renderer;
        std::string m_path;

        void process_events();
        void render();

        // Callbacks for GLFW events
        void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
        void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height);
    };
} // namespace gui

// Implementation
namespace gui
{
    c_window::c_window(int width, int height, const std::string &title)
        : m_window(nullptr, &glfwDestroyWindow),
          m_mesh(m_vertices, m_indices)
    {
        m_window = std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)>(glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr), &glfwDestroyWindow);
        if (!m_window)
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

        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(opengl::gl_debug_callback_fn, nullptr);

        m_mesh.setup_mesh();
        m_shader = std::make_unique<opengl::c_shader>(SOURCE_DIR "/src/shaders/frequency_shader.glsl");
        m_text_renderer.init(width, height);
        m_text_renderer.load_font(SOURCE_DIR "/assets/fonts/Roboto.ttf", 24);

        m_track1 = std::make_shared<music::c_track>();
        m_track2 = std::make_shared<music::c_track>();

        m_track1->play_file("bot.mp3");
        m_track2->play_file("song.mp3");

        m_audio_manager.add_track(m_track1);
        m_audio_manager.add_track(m_track2);

        m_track1->play();
        m_track2->play();
        // std::print("Enter the path to the music file: ");
        // std::cin >> m_path;
        m_audio_samples.resize(1 << 12);
    }

    c_window::~c_window()
    {
    }

    void c_window::show()
    {
        process_events();
        while (not glfwWindowShouldClose(m_window.get()))
        {
            glfwPollEvents();
            render();
        }
    }
    void c_window::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_SPACE)
            {
                // if (m_track->is_playing())
                // {
                //     m_track->pause();
                // }
                // else
                // {
                //     m_track->play();
                // }
            }
            if (key == GLFW_KEY_ESCAPE)
            {
                glfwSetWindowShouldClose(window, 1);
            }
        }
    }

    void c_window::framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height)
    {
        glViewport(0, 0, width, height);
        m_text_renderer.resize(width, height);
    }

    void c_window::process_events()
    {
        auto key_callback = [](GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            static_cast<c_window *>(glfwGetWindowUserPointer(window))->key_callback(window, key, scancode, action, mods);
        };

        auto framebuffer_size_callback = [](GLFWwindow *window, int width, int height)
        {
            static_cast<c_window *>(glfwGetWindowUserPointer(window))->framebuffer_size_callback(window, width, height);
        };

        glfwSetKeyCallback(m_window.get(), key_callback);
        glfwSetFramebufferSizeCallback(m_window.get(), framebuffer_size_callback);
    }

    void c_window::render()
    {
        int window_width = 0;
        int window_height = 0;
        glfwGetFramebufferSize(m_window.get(), &window_width, &window_height);
        m_text_renderer.submit("", (window_width / 2) - 3, window_height / 2, 1.F, glm::vec3{ 1.F, 1.F, 1.F });
        // m_text_renderer.submit(m_track->get_filename(), window_width / 2, window_height / 2 - 25, 1.F, glm::vec3{ 1.F, 1.F, 1.F });
        // std::vector<float> current_frame_audio_samples = m_track->get_samples();
        // std::memmove(m_audio_samples.data(), m_audio_samples.data() + (m_audio_samples.size() - current_frame_audio_samples.size()), (m_audio_samples.size() - current_frame_audio_samples.size()) * sizeof(float));
        // std::memmove(m_audio_samples.data() + (m_audio_samples.size() - current_frame_audio_samples.size()), current_frame_audio_samples.data(), current_frame_audio_samples.size() * sizeof(float));
        //
        // auto frame_audio_sample_fft = math::fft(m_audio_samples)
        //                               | std::views::transform([](auto &&datum)
        //                                                       { return std::abs(datum.real()); })
        //                               | std::ranges::to<std::vector>();
        // // m_max_intensity = std::max(*std::ranges::max_element(frame_audio_sample_fft), m_max_intensity);
        // m_max_intensity = std::ranges::max(frame_audio_sample_fft) + 1.F; // Avoid division by zero
        //
        // auto base_freq = 20.F;
        // float freq = base_freq;
        // auto count = static_cast<std::size_t>(std::floor(12 * (std::log2(static_cast<float>(frame_audio_sample_fft.size())) - std::log2(base_freq)))) + 1; // +1 to include the base frequency
        //
        // m_vertices.resize(count * 4); // A rectangle bar = 4 vertices
        // m_indices.resize(count * 6);  // 6 indices per rectangle (2 triangles, i.e. vertex (1,2,3) and (2,4,3))
        //
        // for (std::size_t index{}; freq < (float)frame_audio_sample_fft.size(); freq *= static_cast<float>(std::pow(2, 1.F / 12.F)), ++index)
        // {
        //     auto value = frame_audio_sample_fft[static_cast<std::size_t>(std::floor(freq))];
        //     float x_base = window_width / count * index;
        //     float y_base = window_height / m_max_intensity * value;
        //     m_vertices[(std::size_t)(index * 4) + 0] = opengl::s_vertex{ glm::vec3{ x_base, 0, 0 } };
        //     m_vertices[(std::size_t)(index * 4) + 1] = opengl::s_vertex{ glm::vec3{ (x_base + (window_width / count)), 0, 0 } };
        //     m_vertices[(std::size_t)(index * 4) + 2] = opengl::s_vertex{ glm::vec3{ x_base, y_base, 0 } };
        //     m_vertices[(std::size_t)(index * 4) + 3] = opengl::s_vertex{ glm::vec3{ (x_base + (window_width / count)), y_base, 0 } };
        //     m_indices[(std::size_t)(index * 6) + 0] = (unsigned)index * 4 + 0;
        //     m_indices[(std::size_t)(index * 6) + 1] = (unsigned)index * 4 + 1;
        //     m_indices[(std::size_t)(index * 6) + 2] = (unsigned)index * 4 + 2;
        //     m_indices[(std::size_t)(index * 6) + 3] = (unsigned)index * 4 + 2;
        //     m_indices[(std::size_t)(index * 6) + 4] = (unsigned)index * 4 + 1;
        //     m_indices[(std::size_t)(index * 6) + 5] = (unsigned)index * 4 + 3;
        // }
        // auto projection = glm::gtc::ortho(0.F, static_cast<float>(window_width), 0.F, static_cast<float>(window_height), -1.F, 1.F);
        // m_shader->set_uniform_mat4f("projection", projection);
        // m_shader->set_uniform_1f("u_screenWidth", static_cast<float>(window_width));
        // m_mesh.update_mesh(m_vertices, m_indices);
        // m_mesh.draw(m_renderer, *m_shader);
        m_text_renderer.draw_texts();
        glfwSwapBuffers(m_window.get());
    }
} // namespace gui
