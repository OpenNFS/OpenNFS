#include "UIRenderer.h"

#include "../UI/UIButton.h"
#include "../UI/UIImage.h"
#include "../UI/UITextField.h"

#include <algorithm>
#include <freetype/freetype.h>
#include <glm/gtc/matrix_transform.hpp>

namespace OpenNFS {
    UIRenderer::UIRenderer() {
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
    }

    bool UIRenderer::GenerateAtlases(std::map<std::string, UIFont> const &fontMap) {
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            LOG(WARNING) << "Failed to initialize FreeType library";
            return false;
        }

        // Load fonts into renderer
        for (auto const &[name, font] : fontMap) {
            Atlas newAtlas;
            bool const success = newAtlas.Initialise(ft, font.path, font.size);

            if (success) {
                m_fontAtlases[name] = std::move(newAtlas);
                LOG(INFO) << "Loaded font atlas: " << name << " from " << font.path << " (size: " << font.size << ")";
                continue;
            }

            LOG(WARNING) << "Failed to load font atlas: " << name << " from " << font.path;
            return false;
        }
        FT_Done_FreeType(ft);

        return true;
    }

    UIRenderer::~UIRenderer() {
    }

    void UIRenderer::BeginRenderPass() {
        m_projectionMatrix = glm::ortho(0.0f, static_cast<float>(Config::get().resX), 0.0f, static_cast<float>(Config::get().resY));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Allow for hot reload of shaders
        m_uiShader.HotReload();
        m_fontShader.HotReload();
    }

    void UIRenderer::EndRenderPass() {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    void UIRenderer::RenderButton(UIButton const *button) const {
        RenderResource(button->resource, static_cast<GLint>(button->layer), button->location.x, button->location.y, button->scale);
        RenderText(button->text, button->fontName, static_cast<GLint>(button->layer), button->location.x + button->textOffset.x,
                   button->location.y + button->textOffset.y, button->scale, button->textColour, true);
    }

    void UIRenderer::RenderTextField(UITextField const *textField) const {
        RenderText(textField->text, textField->fontName, static_cast<GLint>(textField->layer), textField->location.x, textField->location.y,
                   textField->scale, textField->textColour, false);
    }

    void UIRenderer::RenderImage(UIImage const *image) const {
        RenderResource(image->resource, static_cast<GLint>(image->layer), image->location.x, image->location.y, image->scale);
    }

    void UIRenderer::RenderText(std::string const &text, std::string const &fontName, GLint const layer, GLfloat x, GLfloat const y,
                                GLfloat const scale, glm::vec4 const colour, bool isButtonText) const {
        CHECK_F(layer >= 0 && layer <= 200, "Layer: %d is outside of range 0-200", layer);

        // Get the font atlas
        auto const it = m_fontAtlases.find(fontName);
        if (it == m_fontAtlases.end()) {
            LOG(WARNING) << "Font not found: " << fontName << ", skipping text render";
            return;
        }
        Atlas const &atlas = it->second;

        // Activate the corresponding render state
        m_fontShader.use();
        m_fontShader.loadLayer(layer, isButtonText);
        m_fontShader.loadColour(colour);
        m_fontShader.loadProjectionMatrix(m_projectionMatrix);

        // Bind the atlas texture once
        m_fontShader.loadGlyphTexture(atlas.GetTextureID());

        glBindVertexArray(atlas.GetVAO());
        // Iterate through all characters
        for (auto c{text.begin()}; c != text.end(); ++c) {
            unsigned char const ch_idx = static_cast<unsigned char>(*c);
            auto const &ch = atlas.GetCharacter(ch_idx);

            // Skip glyphs with no pixels
            if (ch.bw == 0 || ch.bh == 0) {
                x += ch.ax * scale;
                continue;
            }

            GLfloat const xpos = x + ch.bl * scale;
            GLfloat const ypos = y - (ch.bh - ch.bt) * scale;
            GLfloat const w = ch.bw * scale;
            GLfloat const h = ch.bh * scale;

            // Calculate texture coordinates from atlas
            GLfloat const tx = ch.tx;
            GLfloat const ty = ch.ty;
            GLfloat const tx2 = tx + ch.bw / static_cast<float>(atlas.GetWidth());
            GLfloat const ty2 = ty + ch.bh / static_cast<float>(atlas.GetHeight());

            // Update VBO for each character with atlas texture coordinates
            GLfloat const vertices[6][4] = {{xpos, ypos + h, tx, ty}, {xpos, ypos, tx, ty2},      {xpos + w, ypos, tx2, ty2},
                                            {xpos, ypos + h, tx, ty}, {xpos + w, ypos, tx2, ty2}, {xpos + w, ypos + h, tx2, ty}};

            // Update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, atlas.GetVBO());
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // Advance cursor for the next glyph
            x += ch.ax * scale;
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_fontShader.unbind();
    }

    void UIRenderer::RenderResource(UIResource const &resource, GLint const layer, GLfloat const x, GLfloat const y,
                                    GLfloat const scale) const {
        // TODO: Actually implement this rescaling scaling properly
        constexpr float ratioX = 1.0f; // (float) resource.width / Config::get().resX;
        constexpr float ratioY = 1.0f; // (float) resource.height / Config::get().resY;

        RenderResource(resource, layer, x, y, ratioX * resource.width, ratioY * resource.height, scale);
    }

    void UIRenderer::RenderResource(UIResource const &resource, GLint const layer, GLfloat const x, GLfloat const y, GLfloat const width,
                                    GLfloat const height, GLfloat const scale) const {
        CHECK_F(layer >= 0 && layer <= 200, "Layer: %d is outside of range 0-200", layer);

        GLfloat const xpos{x};
        GLfloat const ypos{y};
        // Iterate through all characters
        GLfloat const w{width * scale};
        GLfloat const h{height * scale};
        // Update VBO
        GLfloat const vertices[6][4]{{xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},     {xpos + w, ypos, 1.0, 1.0},
                                     {xpos, ypos + h, 0.0, 0.0}, {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0}};

        // Activate the corresponding render state
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

    UIRenderer::Atlas::~Atlas() {
        if (m_fontAtlasTexture != 0) {
            glDeleteTextures(1, &m_fontAtlasTexture);
        }
        if (m_fontQuadVAO != 0) {
            glDeleteVertexArrays(1, &m_fontQuadVAO);
        }
        if (m_fontQuadVBO != 0) {
            glDeleteBuffers(1, &m_fontQuadVBO);
        }
    }

    UIRenderer::Atlas::Atlas(Atlas &&other) noexcept
        : m_fontQuadVAO(other.m_fontQuadVAO), m_fontQuadVBO(other.m_fontQuadVBO), m_fontAtlasTexture(other.m_fontAtlasTexture),
          m_atlasWidth(other.m_atlasWidth), m_atlasHeight(other.m_atlasHeight), m_characters(other.m_characters) {
        // Transfer ownership - set other's handles to 0 so destructor won't delete them
        other.m_fontQuadVAO = 0;
        other.m_fontQuadVBO = 0;
        other.m_fontAtlasTexture = 0;
    }

    UIRenderer::Atlas &UIRenderer::Atlas::operator=(Atlas &&other) noexcept {
        if (this != &other) {
            // Clean up our own resources first
            if (m_fontAtlasTexture != 0) {
                glDeleteTextures(1, &m_fontAtlasTexture);
            }
            if (m_fontQuadVAO != 0) {
                glDeleteVertexArrays(1, &m_fontQuadVAO);
            }
            if (m_fontQuadVBO != 0) {
                glDeleteBuffers(1, &m_fontQuadVBO);
            }

            // Transfer ownership
            m_fontQuadVAO = other.m_fontQuadVAO;
            m_fontQuadVBO = other.m_fontQuadVBO;
            m_fontAtlasTexture = other.m_fontAtlasTexture;
            m_atlasWidth = other.m_atlasWidth;
            m_atlasHeight = other.m_atlasHeight;
            m_characters = other.m_characters;

            // Set other's handles to 0
            other.m_fontQuadVAO = 0;
            other.m_fontQuadVBO = 0;
            other.m_fontAtlasTexture = 0;
        }
        return *this;
    }

    bool UIRenderer::Atlas::Initialise(FT_Library const &ft, std::string const &fontPath, int const size) {
        FT_Face face;
        CHECK_F(!FT_New_Face(ft, fontPath.c_str(), 0, &face), "FREETYPE: Failed to load font");

        FT_Set_Pixel_Sizes(face, 0, size);
        FT_GlyphSlot const g = face->glyph;

        // First pass: calculate atlas dimensions
        unsigned int row_w = 0;
        unsigned int row_h = 0;
        m_atlasWidth = 0;
        m_atlasHeight = 0;

        for (int i = 32; i < 128; i++) {
            if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
                LOG(WARNING) << "FREETYPE: Failed to load character " << static_cast<char>(i);
                return false;
            }
            if (row_w + g->bitmap.width + 1 >= MAX_WIDTH) {
                m_atlasWidth = std::max(m_atlasWidth, row_w);
                m_atlasHeight += row_h;
                row_w = 0;
                row_h = 0;
            }
            row_w += g->bitmap.width + 1;
            row_h = std::max(row_h, g->bitmap.rows);
        }

        m_atlasWidth = std::max(m_atlasWidth, row_w);
        m_atlasHeight += row_h;

        // Create atlas texture
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &m_fontAtlasTexture);
        glBindTexture(GL_TEXTURE_2D, m_fontAtlasTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_atlasWidth, m_atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

        // Set texture parameters
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Second pass: paste glyphs into atlas and store character info
        int ox = 0;
        int oy = 0;
        row_h = 0;

        for (int i = 32; i < 128; i++) {
            if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
                LOG(WARNING) << "FREETYPE: Failed to load character " << static_cast<char>(i);
                return false;
            }

            if (ox + g->bitmap.width + 1 >= MAX_WIDTH) {
                oy += row_h;
                row_h = 0;
                ox = 0;
            }

            glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

            m_characters[i].ax = g->advance.x >> 6;
            m_characters[i].ay = g->advance.y >> 6;
            m_characters[i].bw = g->bitmap.width;
            m_characters[i].bh = g->bitmap.rows;
            m_characters[i].bl = g->bitmap_left;
            m_characters[i].bt = g->bitmap_top;
            m_characters[i].tx = ox / static_cast<float>(m_atlasWidth);
            m_characters[i].ty = oy / static_cast<float>(m_atlasHeight);

            row_h = std::max(row_h, g->bitmap.rows);
            ox += g->bitmap.width + 1;
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        FT_Done_Face(face);

        LOG(INFO) << "Font atlas created: " << m_atlasWidth << "x" << m_atlasHeight << " (" << (m_atlasWidth * m_atlasHeight / 1024)
                  << " kb)";

        // Configure VAO/VBO for font texture quads
        glGenVertexArrays(1, &m_fontQuadVAO);
        glGenBuffers(1, &m_fontQuadVBO);
        glBindVertexArray(m_fontQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_fontQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);

        return true;
    }

    GLuint UIRenderer::Atlas::GetTextureID() const {
        return m_fontAtlasTexture;
    }

    uint32_t UIRenderer::Atlas::GetWidth() const {
        return m_atlasWidth;
    }

    uint32_t UIRenderer::Atlas::GetHeight() const {
        return m_atlasHeight;
    }

    GLuint UIRenderer::Atlas::GetVAO() const {
        return m_fontQuadVAO;
    }

    GLuint UIRenderer::Atlas::GetVBO() const {
        return m_fontQuadVBO;
    }

    UIRenderer::Atlas::CharacterInfo UIRenderer::Atlas::GetCharacter(uint8_t const charIdx) const {
        return m_characters.at(charIdx);
    }
} // namespace OpenNFS
