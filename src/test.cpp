#include <GL/glew.h>

#include <SFML/Window.hpp>

#include <chrono>
#include <cmath>
#include <expected>
#include <print>
#include <ranges>
#include <vector>

import opengl;
import music;
import fft;

using namespace std::literals::chrono_literals;

int main()
{
    sf::ContextSettings settings;
    settings.majorVersion = 4;
    settings.minorVersion = 5;
    sf::Window window(sf::VideoMode(800, 600), "OpenGL", sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);

    if (glewInit() != GLEW_OK)
    {
        std::println("Error initializing GLEW");
        return -1;
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Ensure closer fragments overwrite farther ones

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageCallback(opengl::gl_debug_callback_fn, nullptr);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    opengl::c_vertex_array vao;
    opengl::c_buffer_layout layout;
    layout.push<float>(2);

    opengl::c_shader shader("../src/shaders/basic.shader");

    opengl::c_renderer renderer;

    shader.unbind();
    vao.unbind();

    // INFO: Trials here!
    music::c_track track("see-you-later-203103.mp3");
    track.play();

    bool running = true;
    while (running)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                running = false;
            }
            if (event.type == sf::Event::Resized)
            {
                glViewport(0, 0, event.size.width, event.size.height);
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Q)
            {
                running = false;
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
            {
                if (track.is_playing())
                {
                    track.pause();
                }
                else
                {
                    track.play();
                }
            }
        }

        auto start = std::chrono::high_resolution_clock::now();

        auto [width, height] = window.getSize();
        auto data = track.get_samples();
        auto fft_data = math::fft(data)
                        | std::views::transform([](auto &&datum)
                                                { return datum.real(); })
                        | std::ranges::to<std::vector>();
        auto max = *std::ranges::max_element(fft_data);

        float base_freq = 20.F;
        std::size_t size = 12 * (std::log2(fft_data.size()) - std::log2(base_freq));
        indices.resize(fft_data.size() * 6);
        vertices.resize(fft_data.size() * 4 * 2);
        for (std::size_t index{}, freq = base_freq; freq < fft_data.size(); ++index, freq *= std::pow(2, 1.F / 12.F))
        {
            auto value = fft_data[std::floor(freq)];
            // for (auto [index, value] : fft_data | std::views::enumerate)
            // {
            float x0 = (float)width / size * index;
            float y0 = (float)height / max * value;
            vertices[(std::size_t)(index * 4 * 2) + 0] = x0;
            vertices[(std::size_t)(index * 4 * 2) + 1] = 0;
            vertices[(std::size_t)(index * 4 * 2) + 2] = (x0 + (float)(width / size));
            vertices[(std::size_t)(index * 4 * 2) + 3] = 0;
            vertices[(std::size_t)(index * 4 * 2) + 4] = x0;
            vertices[(std::size_t)(index * 4 * 2) + 5] = y0;
            vertices[(std::size_t)(index * 4 * 2) + 6] = (x0 + (float)(width / size));
            vertices[(std::size_t)(index * 4 * 2) + 7] = y0;
            indices[(std::size_t)(index * 6) + 0] = (unsigned)index * 4 + 0;
            indices[(std::size_t)(index * 6) + 1] = (unsigned)index * 4 + 1;
            indices[(std::size_t)(index * 6) + 2] = (unsigned)index * 4 + 2;
            indices[(std::size_t)(index * 6) + 3] = (unsigned)index * 4 + 2;
            indices[(std::size_t)(index * 6) + 4] = (unsigned)index * 4 + 1;
            indices[(std::size_t)(index * 6) + 5] = (unsigned)index * 4 + 3;
        }

        opengl::c_vertex_buffer vbuff(vertices, (unsigned)vertices.size() * sizeof(float));
        opengl::c_index_buffer ibuff(indices, (unsigned)indices.size());

        auto mat = glm::gtc::ortho(0.F, (float)width, 0.F, (float)height, -1.F, 1.F);

        renderer.clear();
        shader.bind();
        shader.set_uniform_mat4f("projection", mat);
        vao.bind();
        ibuff.bind();
        vbuff.bind();
        vao.add_buffer(vbuff, layout);
        renderer.draw(vao, ibuff, shader);

        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() < 1000 / 60.F)
        {
        }
        window.display();
    }
    return 0;
}
