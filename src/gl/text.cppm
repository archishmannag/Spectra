module;

#include <GL/glew.h>
#include <ft2build.h>

#include FT_FREETYPE_H

#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
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
        std::map<std::uint64_t, s_character> m_character_map; // Map of character to character map
        glm::mat4 m_projection_matrix{};                      // Projection matrix for text rendering

        std::unique_ptr<c_vertex_array> m_vao;
        std::unique_ptr<c_vertex_buffer> m_vbo;
        std::unique_ptr<c_shader> m_shader;

    public:
        c_text_renderer() = default;
        ~c_text_renderer() = default;

        /**
         * Initialize text renderer with the given width and height.
         * @param width Width of the rendering area
         * @param height Height of the rendering area
         */
        void init(int width, int height);
        void load_font(const std::filesystem::path &font_path, unsigned int font_size);
        void resize(int width, int height);
        void submit(const std::string &text, float x_pos, float y_pos, float scale, const glm::vec3 &color);
        void draw_texts();

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

    void c_text_renderer::load_font(const std::filesystem::path &font_path, unsigned int font_size)
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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            m_character_map[charcode] = {
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

    void c_text_renderer::resize(int width, int height)
    {
        m_projection_matrix = glm::gtc::ortho(0.F, static_cast<float>(width), 0.F, static_cast<float>(height), -1.F, 1.F);
        m_shader->set_uniform_mat4f("projection", m_projection_matrix);
    }

    void c_text_renderer::submit(const std::string &text, float x_pos, float y_pos, float scale, const glm::vec3 &color)
    {
        if (text.empty())
        {
            return;
        }
        std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety
        m_text_draw_queue.emplace_back(text, x_pos, y_pos, scale, color);
    }

    void c_text_renderer::draw_texts()
    {

        if (m_text_draw_queue.empty())
        {
            return;
        }

        std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety
        glActiveTexture(GL_TEXTURE0);
        m_vao->bind();
        for (auto &[text, x, y, scale, color] : m_text_draw_queue)
        {
            m_shader->set_uniform_3f("textColor", color);
            m_shader->bind();

            for (const char character : text)
            {
                if (m_character_map.find(character) == m_character_map.end())
                {
                    std::println(std::cerr, "Character '{}' not found in character map", character);
                    continue;
                }

                s_character character_struct = m_character_map[character];
                float xpos = x + character_struct.bearing.x * scale;
                float ypos = y - (character_struct.size.y - character_struct.bearing.y) * scale;
                float w = character_struct.size.x * scale;
                float h = character_struct.size.y * scale;

                // Only render if the character has dimensions
                if (w <= 0 || h <= 0)
                {
                    x += (character_struct.advance >> 6) * scale; // Still advance cursor
                    continue;
                }

                // clang-format off
                // Update VBO for each character
                // Remember that Gl's co-ordinate has y-axis increasing bottom to top, but texture coordinates have y-axis increasing top to bottom.
                // So we flip texture's y-coordinate (0->1, 1->0)
                std::vector<float> vertices{
                    xpos,     ypos,     0.0F, 1.0F,
                    xpos,     ypos + h, 0.0F, 0.0F,
                    xpos + w, ypos + h, 1.0F, 0.0F,
                    
                    xpos + w, ypos,     1.0F, 1.0F,
                    xpos + w, ypos + h, 1.0F, 0.0F, 
                    xpos,    ypos,      0.0F, 1.0F
                };
                // clang-format on

                glBindTexture(GL_TEXTURE_2D, character_struct.texture_id);
                m_vbo->update_buffer(vertices, 0, vertices.size() * sizeof(float));

                // Draw
                m_vbo->bind();
                glDrawArrays(GL_TRIANGLES, 0, 6);

                // Advance cursor for next glyph
                x += (character_struct.advance >> 6) * scale;
            }
        }
        m_vao->unbind();
        glBindTexture(GL_TEXTURE_2D, 0);
        m_shader->unbind();
        m_text_draw_queue.clear();
    }

} // namespace opengl
