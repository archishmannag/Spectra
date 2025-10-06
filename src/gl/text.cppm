module;
#include <GL/glew.h>
#include <ft2build.h>
#include <utf8.h>

#include FT_FREETYPE_H

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <map>
#include <mutex>
#include <print>
#include <string>
#include <string_view>
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
        ~c_text_renderer();

        auto load_font(const std::filesystem::path &font_path, unsigned int font_size) -> void;
        auto resize(glm::vec2 new_size) -> void;
        auto submit(const std::string &text, glm::vec2 position, float scale, const glm::vec3 &color) -> void;
        auto draw_texts() -> void;
        auto get_size(std::string_view text, float scale) const -> glm::vec2;

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

    c_text_renderer::~c_text_renderer()
    {
        for (auto &[_, character] : m_character_map)
        {
            glDeleteTextures(1, &character.texture_id);
        }
    }

    auto c_text_renderer::load_font(const std::filesystem::path &font_path, unsigned int font_size) -> void
    {
        // Clear existing character map and textures
        for (auto &[_, character] : m_character_map)
        {
            glDeleteTextures(1, &character.texture_id);
        }
        m_character_map.clear();

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

    auto c_text_renderer::resize(glm::vec2 new_size) -> void
    {
        m_projection_matrix = glm::gtc::ortho(0.F, new_size.x, 0.F, new_size.y, -1.F, 1.F);
        m_shader.set_uniform_mat4f("projection", m_projection_matrix);
    }

    auto c_text_renderer::submit(const std::string &text, glm::vec2 position, float scale, const glm::vec3 &color) -> void
    {
        if (text.empty())
        {
            return;
        }
        std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety
        m_text_draw_queue.emplace_back(text, position.x, position.y, scale, color);
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
                if (char_width <= 0 or char_height <= 0)
                {
                    x += static_cast<float>(character_struct.advance >> 6U) * scale; // Still advance cursor
                    continue;
                }

                // clang-format off
                // Update VBO for each character
                // Remember that Gl's co-ordinate has y-axis increasing bottom to top, but texture coordinates have y-axis increasing top to bottom.
                // So we flip texture's y-coordinate (0->1, 1->0)
                std::vector<float> vertices{
                    xpos,              ypos,               0.0F, 1.0F, //    (x,y+h)            (x+w,y+h)
                    xpos,              ypos + char_height, 0.0F, 0.0F, //    (0,0)              (0,1)
                    xpos + char_width, ypos + char_height, 1.0F, 0.0F, //      +------------------+
                    //                                                         |                  |
                    xpos + char_width, ypos,               1.0F, 1.0F, //      +------------------+
                    xpos + char_width, ypos + char_height, 1.0F, 0.0F, //    (1,1)              (1,0)
                    xpos,              ypos,               0.0F, 1.0F  //    (x,y)              (x+w,y)
                };
                // clang-format on

                glBindTexture(GL_TEXTURE_2D, character_struct.texture_id);
                m_vbo.update_buffer(vertices, vertices.size());

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

    auto c_text_renderer::get_size(std::string_view text, float scale) const -> glm::vec2
    {
        if (text.empty())
        {
            return { 0.F, 0.F };
        }

        float width = 0.F;
        float height = 0.F;
        float max_bearing_y = 0.F;
        float max_descender = 0.F;

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

            const s_character &char_struct = m_character_map.at(character);
            width += (static_cast<float>(char_struct.advance >> 6U)) * scale; // Advance is in 1/64 pixels

            float bearing_y = static_cast<float>(char_struct.bearing.y) * scale;
            max_bearing_y = std::max(bearing_y, max_bearing_y);

            float descender = static_cast<float>(char_struct.size.y - char_struct.bearing.y) * scale;
            max_descender = std::max(descender, max_descender);
        }

        height = max_bearing_y + max_descender;

        return { width, height };
    }
} // namespace opengl
