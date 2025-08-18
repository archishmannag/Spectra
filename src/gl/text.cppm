module;
#include <GL/glew.h>
#include <ft2build.h>
#include <utf8.h>

#include FT_FREETYPE_H

#include <filesystem>
#include <iostream>
#include <map>
#include <mutex>
#include <print>
#include <string>
#include <vector>
export module opengl:text;

import :shader;
import :vertex_array;
import :index_buffer;
import :vertex_buffer;
import :buffer_layout;

import utility;
import glm;

namespace
{
    struct s_character
    {
        unsigned int texture_id; // ID handle of the glyph texture
        glm::ivec2 size;         // Size of glyph
        glm::ivec2 bearing;      // Offset from baseline to left/top of glyph
        unsigned int advance;    // Horizontal offset to advance to next glyph
    };

    struct s_text_draw_data
    {
        std::string text; // The text to render
        float x;          // X position to start rendering
        float y;          // Y position to start rendering
        float scale;      // Scale factor for the text
        glm::vec3 color;  // Color of the text
    };

} // namespace

export namespace opengl
{

    class c_text_renderer
    {
    private:
        std::mutex m_mutex;                                   // Mutex for thread safety
        std::vector<s_text_draw_data> m_text_draw_queue;      // Queue of text draw data
        std::map<std::uint32_t, s_character> m_character_map; // Map of character to character map
        glm::mat4 m_projection_matrix{};                      // Projection matrix for text rendering

        c_vertex_array m_vao;
        c_vertex_buffer<float> m_vbo;
        c_shader m_shader;

    public:
        c_text_renderer(int width, int height);
        ~c_text_renderer() = default;

        auto load_font(const std::filesystem::path &font_path, unsigned int font_size) -> void;
        auto resize(int width, int height) -> void;
        auto submit(const std::string &text, float x_pos, float y_pos, float scale, const glm::vec3 &color) -> void;
        auto draw_texts() -> void;

        // Disable copy and move semantics
        c_text_renderer(const c_text_renderer &) = delete;
        c_text_renderer(c_text_renderer &&) = delete;
        auto operator=(const c_text_renderer &) -> c_text_renderer & = delete;
        auto operator=(c_text_renderer &&) -> c_text_renderer & = delete;
    };

} // namespace opengl

// Implementation
namespace opengl
{
    c_text_renderer::c_text_renderer(int width, int height)
        : m_vbo(std::vector<float>{}, 6UL * 4 * sizeof(float), GL_DYNAMIC_DRAW),
          m_shader(SOURCE_DIR "/src/shaders/text_shader.glsl")
    {
        m_projection_matrix = glm::gtc::ortho(0.F, static_cast<float>(width), 0.F, static_cast<float>(height));

        auto init = [this]()
        {
            m_shader.set_uniform_mat4f("projection", m_projection_matrix);
            m_shader.set_uniform_1i("text", 0); // Set the texture sampler to 0

            c_buffer_layout layout;
            layout.push<float>(2); // Position
            layout.push<float>(2); // Texture coordinates

            m_vao.add_buffer(m_vbo, layout);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        };

        utility::c_notifier::subscribe(init);
    }

    auto c_text_renderer::load_font(const std::filesystem::path &font_path, unsigned int font_size) -> void
    {
        FT_Library ft_lib{};
        if (FT_Init_FreeType(&ft_lib))
        {
            std::println(std::cerr, "Could not initialize FreeType library");
            return;
        }

        FT_Face face{};
        if (FT_New_Face(ft_lib, font_path.c_str(), 0, &face))
        {
            std::println(std::cerr, "Could not load font: {}", font_path.c_str());
            return;
        }

        FT_Select_Charmap(face, FT_ENCODING_UNICODE);

        FT_Set_Pixel_Sizes(face, 0, font_size);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

        // Load all unicode characters
        std::uint32_t glyph_index{};
        std::uint64_t charcode = FT_Get_First_Char(face, &glyph_index);
        while (glyph_index)
        {
            if (FT_Load_Char(face, charcode, FT_LOAD_RENDER))
            {
                std::println(std::cerr, "Could not load character '{}'", charcode);
                continue;
            }

            unsigned int texture_id{};
            glGenTextures(1, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<int>(face->glyph->bitmap.width), static_cast<int>(face->glyph->bitmap.rows), 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            m_character_map[static_cast<std::uint32_t>(charcode)] = {
                .texture_id = texture_id,
                .size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                .bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                .advance = static_cast<unsigned int>(face->glyph->advance.x)
            };
            charcode = FT_Get_Next_Char(face, charcode, &glyph_index);
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft_lib);
    }

    auto c_text_renderer::resize(int width, int height) -> void
    {
        m_projection_matrix = glm::gtc::ortho(0.F, static_cast<float>(width), 0.F, static_cast<float>(height), -1.F, 1.F);
        m_shader.set_uniform_mat4f("projection", m_projection_matrix);
    }

    auto c_text_renderer::submit(const std::string &text, float x_pos, float y_pos, float scale, const glm::vec3 &color) -> void
    {
        if (text.empty())
        {
            return;
        }
        std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety
        m_text_draw_queue.emplace_back(text, x_pos, y_pos, scale, color);
    }

    auto c_text_renderer::draw_texts() -> void
    {
        if (m_text_draw_queue.empty())
        {
            return;
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        glActiveTexture(GL_TEXTURE0);
        m_vao.bind();
        for (auto &[text, x, y, scale, color] : m_text_draw_queue)
        {
            m_shader.set_uniform_3f("textColor", color);
            m_shader.bind();

            // We process text to ensure it is valid UTF-8
            std::string temp = utf8::replace_invalid(text);
            auto iter = temp.begin();

            for (auto _ = 0; _ < utf8::distance(temp.begin(), temp.end()); ++_)
            {
                std::uint32_t character = utf8::next(iter, temp.end());
                if (not m_character_map.contains(character))
                {
                    std::println(std::cerr, "Character '{}' not found in character map", character);
                    continue;
                }

                s_character character_struct = m_character_map[character];
                float xpos = x + (static_cast<float>(character_struct.bearing.x) * scale);
                float ypos = y - (static_cast<float>(character_struct.size.y - character_struct.bearing.y) * scale);
                float char_width = static_cast<float>(character_struct.size.x) * scale;
                float char_height = static_cast<float>(character_struct.size.y) * scale;

                // Only render if the character has dimensions
                if (char_width <= 0 || char_height <= 0)
                {
                    x += static_cast<float>(character_struct.advance >> 6U) * scale; // Still advance cursor
                    continue;
                }

                // clang-format off
                // Update VBO for each character
                // Remember that Gl's co-ordinate has y-axis increasing bottom to top, but texture coordinates have y-axis increasing top to bottom.
                // So we flip texture's y-coordinate (0->1, 1->0)
                std::vector<float> vertices{
                    xpos,     ypos,     0.0F, 1.0F,
                    xpos,     ypos + char_height, 0.0F, 0.0F,
                    xpos + char_width, ypos + char_height, 1.0F, 0.0F,
                    
                    xpos + char_width, ypos,     1.0F, 1.0F,
                    xpos + char_width, ypos + char_height, 1.0F, 0.0F, 
                    xpos,    ypos,      0.0F, 1.0F
                };
                // clang-format on

                glBindTexture(GL_TEXTURE_2D, character_struct.texture_id);
                m_vbo.update_buffer(vertices, 0, vertices.size());

                // Draw
                m_vbo.bind();
                glDrawArrays(GL_TRIANGLES, 0, 6);

                // Advance cursor for next glyph
                x += static_cast<float>(character_struct.advance >> 6U) * scale;
            }
        }
        m_vao.unbind();
        glBindTexture(GL_TEXTURE_2D, 0);
        m_shader.unbind();
        m_text_draw_queue.clear();
    }

} // namespace opengl
