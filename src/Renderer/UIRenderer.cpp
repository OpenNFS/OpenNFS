#include "UIRenderer.h"

#include "../UI/UIButton.h"
#include "../UI/UIImage.h"
#include "../UI/UITextField.h"

#include <freetype/freetype.h>
#include <glm/gtc/matrix_transform.hpp>
#include <json/json.hpp>

namespace OpenNFS {
    UIRenderer::UIRenderer() {
        FT_Library ft;
        CHECK_F(!FT_Init_FreeType(&ft), "FREETYPE: Could not init FreeType Library");

        FT_Face face;
        CHECK_F(!FT_New_Face(ft, "../resources/ui/fonts/earth.ttf", 0, &face), "FREETYPE: Failed to load font");

        FT_Set_Pixel_Sizes(face, 0, 48);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

        for (GLubyte c = 0; c < 128; c++) {
            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                LOG(WARNING) << "FREETYPE: Failed to load Glyph";
                continue;
            }
            // Generate texture
            GLuint textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                         face->glyph->bitmap.buffer);
            // Set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // Now store character for later use
            Character character = {textureID, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                                   glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), (GLuint)face->glyph->advance.x};
            m_characterMap.insert(std::pair<GLchar, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        // Configure VAO/VBO for font texture quads
        glGenVertexArrays(1, &m_fontQuadVAO);
        glGenBuffers(1, &m_fontQuadVBO);
        glBindVertexArray(m_fontQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_fontQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
        // Configure VAO/VBO for menu texture quads
        glGenVertexArrays(1, &m_menuQuadVAO);
        glGenBuffers(1, &m_menuQuadVBO);
        glBindVertexArray(m_menuQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_menuQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
        // Reset state
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        LOG(INFO) << "Glyphs loaded successfully";
    }

    UIRenderer::~UIRenderer() {
        // Delete all the loaded textures
        for (auto &character : m_characterMap | std::views::values) {
            glDeleteTextures(1, &character.textureID);
        }
    }
    void UIRenderer::BeginRenderPass() {
        m_projectionMatrix = glm::ortho(0.0f, (float)Config::get().resX, 0.0f, (float)Config::get().resY);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Allow for hot reload of shaders
        m_uiShader.HotReload();
        m_fontShader.HotReload();
    }
    void UIRenderer::EndRenderPass() {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    void UIRenderer::RenderButton(UIButton const *button) {
        RenderText(button->text, static_cast<GLint>(button->layer), button->location.x, button->location.y, button->scale,
                   button->textColour);
        RenderResource(button->resource, static_cast<GLint>(button->layer), button->location.x, button->location.y, button->scale);
    }

    void UIRenderer::RenderTextField(UITextField const *textField) {
        RenderText(textField->text, static_cast<GLint>(textField->layer), textField->location.x, textField->location.y, textField->scale,
                   textField->textColour);
    }

    void UIRenderer::RenderImage(UIImage const *image) {
        RenderResource(image->resource, static_cast<GLint>(image->layer), image->location.x, image->location.y, image->scale);
    }

    void UIRenderer::RenderText(std::string const &text, GLint const layer, GLfloat x, GLfloat const y, GLfloat const scale, glm::vec3 const colour) {
        CHECK_F(layer >= 0 && layer <= 200, "Layer: %d is outside of range 0-200", layer);

        // Activate corresponding render state
        m_fontShader.use();
        m_fontShader.loadLayer(layer);
        m_fontShader.loadColour(colour);
        m_fontShader.loadProjectionMatrix(m_projectionMatrix);

        glBindVertexArray(m_fontQuadVAO);
        // Iterate through all characters
        for (auto c{text.begin()}; c != text.end(); ++c) {
            Character const ch{m_characterMap[*c]};

            GLfloat const xpos{x + ch.bearing.x * scale};
            GLfloat const ypos{y - (ch.size.y - ch.bearing.y) * scale};

            GLfloat const w{ch.size.x * scale};
            GLfloat const h{ch.size.y * scale};
            // Update VBO for each character
            GLfloat const vertices[6][4]{{xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},     {xpos + w, ypos, 1.0, 1.0},
                                         {xpos, ypos + h, 0.0, 0.0}, {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0}};
            // Render glyph texture over quad
            m_fontShader.loadGlyphTexture(ch.textureID);
            // Update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, m_fontQuadVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_fontShader.unbind();
    }

    void UIRenderer::RenderResource(UIResource const &resource, GLint const layer, GLfloat const x, GLfloat const y, GLfloat const scale) {
        // TODO: Actually implement this rescaling scaling properly
        constexpr float ratioX = 1.0f; // (float) resource.width / Config::get().resX;
        constexpr float ratioY = 1.0f; // (float) resource.height / Config::get().resY;

        RenderResource(resource, layer, x, y, ratioX * resource.width, ratioY * resource.height, scale);
    }

    void UIRenderer::RenderResource(UIResource const &resource, GLint const layer, GLfloat const x, GLfloat const y,
                                    GLfloat const width, GLfloat const height, GLfloat const scale) const {
        CHECK_F(layer >= 0 && layer <= 200, "Layer: %d is outside of range 0-200", layer);

        GLfloat const xpos{x};
        GLfloat const ypos{y};
        // Iterate through all characters
        GLfloat const w{width * scale};
        GLfloat const h{height * scale};
        // Update VBO
        GLfloat const vertices[6][4]{{xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},     {xpos + w, ypos, 1.0, 1.0},
                                     {xpos, ypos + h, 0.0, 0.0}, {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0}};

        // Activate corresponding render state
        m_uiShader.use();
        m_uiShader.loadLayer(layer);
        m_uiShader.loadColour(glm::vec3(1, 1, 1));
        m_uiShader.loadProjectionMatrix(m_projectionMatrix);
        // Render menu texture over quad
        m_uiShader.loadUITexture(resource.textureID);

        glBindVertexArray(m_menuQuadVAO);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, m_menuQuadVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Reset state
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_uiShader.unbind();
    }
} // namespace OpenNFS
