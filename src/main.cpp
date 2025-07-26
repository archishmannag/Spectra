#include <iostream>
#include <print>
#include <stacktrace>

import gui;

int main()
{
    try
    {
        gui::init();
        gui::c_window window(800, 600, "OpenGL");
        window.show();
        gui::terminate();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::println("Exception caught: {}\n{}", e.what(), std::stacktrace::current());
        return -1;
    }
}

// int main()
// {
//     std::string path;
//     std::print("Enter the path to the music file: ");
//     std::cin >> path;
//
//     sf::ContextSettings settings;
//     settings.majorVersion = 4;
//     settings.minorVersion = 5;
//     sf::Window window(sf::VideoMode(800, 600), "OpenGL", sf::Style::Default, settings);
//     window.setVerticalSyncEnabled(true);
//
//     if (glewInit() != GLEW_OK)
//     {
//         std::println("Error initializing GLEW");
//         return -1;
//     }
//
//     // // Enable depth testing
//     // glEnable(GL_DEPTH_TEST);
//     // glDepthFunc(GL_LESS); // Ensure closer fragments overwrite farther ones
//
//     glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
//
//     glDebugMessageCallback(opengl::gl_debug_callback_fn, nullptr);
//
//     opengl::c_vertex_array vao;
//     opengl::c_buffer_layout layout;
//     layout.push<float>(2);
//
//     opengl::c_shader shader("../src/shaders/basic.shader");
//
//     opengl::c_renderer renderer;
//
//     shader.unbind();
//     vao.unbind();
//
//     music::c_track track(path);
//     track.play();
//
//     auto max_freq = 0.F;
//     bool running = true;
//     while (running)
//     {
//         sf::Event event;
//         while (window.pollEvent(event))
//         {
//         }
//
//         auto start = std::chrono::high_resolution_clock::now();
//
//         auto [width, height] = window.getSize();
//         auto data = track.get_samples();
//         data.resize(1 << 13);
//         auto fft_data = math::fft(data)
//                         | std::views::transform([](auto &&datum)
//                                                 { return datum.real(); })
//                         | std::ranges::to<std::vector>();
//         max_freq = std::max(*std::ranges::max_element(fft_data), max_freq);
//         auto base_freq = 10.F;
//         float freq = base_freq;
//         std::size_t size = 12 * (std::log2(fft_data.size()) - std::log2(base_freq));
//         std::vector<float> vertices(fft_data.size() * 4 * 2);
//         std::vector<unsigned> indices(fft_data.size() * 6);
//         for (std::size_t index{}; freq < fft_data.size(); ++index, freq *= std::pow(2, 1.F / 12.F))
//         {
//             auto value = fft_data[std::floor(freq)];
//             float x_base = (float)width / size * index;
//             float y_base = (float)height / max_freq * value;
//             vertices[index * 8 + 0] = x_base;
//             vertices[index * 8 + 1] = 0.F;
//             vertices[index * 8 + 2] = x_base + (float)width / size;
//             vertices[index * 8 + 3] = y_base;
//             vertices[index * 8 + 4] = x_base;
//             vertices[index * 8 + 5] = y_base;
//             vertices[index * 8 + 6] = x_base + (float)width / size;
//             vertices[index * 8 + 7] = 0.F;
//             indices[index * 6 + 0] = index * 4 + 0;
//             indices[index * 6 + 1] = index * 4 + 1;
//             indices[index * 6 + 2] = index * 4 + 2;
//             indices[index * 6 + 3] = index * 4 + 1;
//             indices[index * 6 + 4] = index * 4 + 3;
//             indices[index * 6 + 5] = index * 4 + 2;
//         }
//
//         opengl::c_vertex_buffer vbuff(vertices, (unsigned)vertices.size() * sizeof(float));
//         opengl::c_index_buffer ibuff(indices, (unsigned)indices.size());
//
//         auto mat = glm::gtc::ortho(0.F, (float)width, 0.F, (float)height, -1.F, 1.F);
//
//         renderer.clear();
//         shader.bind();
//         shader.set_uniform_mat4f("projection", mat);
//         vao.bind();
//         ibuff.bind();
//         vbuff.bind();
//         vao.add_buffer(vbuff, layout);
//         renderer.draw(vao, ibuff, shader);
//
//         while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() < 1000 / 60.F)
//         {
//         }
//         window.display();
//     }
//     return 0;
// }
