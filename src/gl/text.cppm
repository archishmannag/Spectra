module;

#include <GL/glew.h>
#include <ft2build.h>

#include FT_FREETYPE_H

#include <iostream>
#include <map>
#include <memory>
#include <print>
#include <string>

export module opengl:text;

import :shader;
import :vertex_array;
import :index_buffer;
import :vertex_buffer;
import :buffer_layout;
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

} // namespace

export namespace opengl
{

    class c_text_renderer
    {
    private:
        std::map<unsigned char, s_character> m_character_map; // Map of character to character map
        glm::mat4 m_projection_matrix{};

        std::unique_ptr<c_vertex_array> m_vao;
        std::unique_ptr<c_vertex_buffer> m_vbo;
        std::unique_ptr<c_shader> m_shader;

    public:
        c_text_renderer() = default;
        ~c_text_renderer() = default;

        void init(int width, int height);
        void load_font(const std::string &font_path, unsigned int font_size);
        void render_text(const std::string &text, float x, float y, float scale, const glm::vec3 &color);
        void resize(int width, int height);

        // Disable copy and move semantics
        c_text_renderer(const c_text_renderer &) = delete;
        c_text_renderer(c_text_renderer &&) = delete;
        c_text_renderer &operator=(const c_text_renderer &) = delete;
        c_text_renderer &operator=(c_text_renderer &&) = delete;
    };

} // namespace opengl

// Implementation
namespace opengl
{
    void c_text_renderer::init(int width, int height)
    {
        m_projection_matrix = glm::gtc::ortho(0.F, static_cast<float>(width), 0.F, static_cast<float>(height));
        m_shader = std::make_unique<c_shader>(SOURCE_DIR "/src/shaders/text_shader.glsl");
        m_shader->set_uniform_mat4f("projection", m_projection_matrix);
        m_shader->set_uniform_1i("text", 0); // Set the texture sampler to 0

        c_buffer_layout layout;
        layout.push<float>(2); // Position
        layout.push<float>(2); // Texture coordinates

        m_vbo = std::make_unique<c_vertex_buffer>(std::vector<float>{}, 6UL * 4 * sizeof(float), GL_DYNAMIC_DRAW);
        m_vao = std::make_unique<c_vertex_array>();
        m_vao->add_buffer(*m_vbo, layout);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void c_text_renderer::load_font(const std::string &font_path, unsigned int font_size)
    {
        FT_Library ft{};
        if (FT_Init_FreeType(&ft))
        {
            std::println(std::cerr, "Could not initialize FreeType library");
            return;
        }

        FT_Face face{};
        if (FT_New_Face(ft, font_path.c_str(), 0, &face))
        {
            std::println(std::cerr, "Could not load font: {}", font_path);
            return;
        }

        FT_Set_Pixel_Sizes(face, 0, font_size);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

        for (unsigned char c = 0; c < 128; ++c)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::println(std::cerr, "Could not load character '{}'", c);
                continue;
            }

            unsigned int texture_id{};
            glGenTextures(1, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            s_character character = {
                .texture_id = texture_id,
                .size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                .bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                .advance = static_cast<unsigned int>(face->glyph->advance.x)
            };
            m_character_map[c] = character;
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

    void c_text_renderer::render_text(const std::string &text, float x, float y, float scale, const glm::vec3 &color)
    {
        if (text.empty())
        {
            return;
        }

        m_shader->set_uniform_3f("textColor", color);
        m_shader->bind();

        glActiveTexture(GL_TEXTURE0);
        m_vao->bind();

        for (const char ch : text)
        {
            if (m_character_map.find(ch) == m_character_map.end())
            {
                std::println(std::cerr, "Character '{}' not found in character map", ch);
                continue;
            }

            s_character character = m_character_map[ch];
            float xpos = x + character.bearing.x * scale;
            float ypos = y - (character.size.y - character.bearing.y) * scale;
            float w = character.size.x * scale;
            float h = character.size.y * scale;

            // Only render if the character has dimensions
            if (w <= 0 || h <= 0)
            {
                x += (character.advance >> 6) * scale; // Still advance cursor
                continue;
            }

            // clang-format off
            // Update VBO for each character
            // Remember that Gl's co-ordinate has y-axis increasing bottom to top, but texture coordinates have y-axis increasing top to bottom, so we flip texture's y-coordinate (0->1, 1->0)
            std::vector<float> vertices = {
                xpos,     ypos,     0.0F, 1.0F,
                xpos,     ypos + h, 0.0F, 0.0F,
                xpos + w, ypos + h, 1.0F, 0.0F,
                
                xpos + w, ypos,     1.0F, 1.0F,
                xpos + w, ypos + h, 1.0F, 0.0F, 
                xpos,    ypos,      0.0F, 1.0F
            };
            // clang-format on

            glBindTexture(GL_TEXTURE_2D, character.texture_id);
            m_vbo->update_buffer(vertices, 0, vertices.size() * sizeof(float));

            // Draw
            m_vbo->bind();
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Advance cursor for next glyph
            x += (character.advance >> 6) * scale;
        }

        m_vao->unbind();
        glBindTexture(GL_TEXTURE_2D, 0);
        m_shader->unbind();
    }

    void c_text_renderer::resize(int width, int height)
    {
        m_projection_matrix = glm::gtc::ortho(0.F, static_cast<float>(width), 0.F, static_cast<float>(height), -1.F, 1.F);
        m_shader->set_uniform_mat4f("projection", m_projection_matrix);
    }
} // namespace opengl
