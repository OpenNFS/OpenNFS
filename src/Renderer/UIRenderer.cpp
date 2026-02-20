#include "UIRenderer.h"

#include "../UI/UIButton.h"
#include "../UI/UIDropdown.h"
#include "../UI/UIImage.h"
#include "../UI/UIShape.h"
#include "../UI/UITextField.h"

#include <algorithm>
#include <cmath>
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

        // Configure VAO/VBO for shape primitives (variable vertex counts so dynamic)
        glGenVertexArrays(1, &m_shapeVAO);
        glGenBuffers(1, &m_shapeVBO);
        glBindVertexArray(m_shapeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_shapeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 3 * 64, nullptr, GL_DYNAMIC_DRAW); // pre-alloc for up to 64 triangles
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);

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
            UIFontAtlas newAtlas;
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
        m_fontShader.cleanup();
        m_uiShader.cleanup();
        m_uiShapeShader.cleanup();
        glDeleteVertexArrays(1, &m_menuQuadVAO);
        glDeleteBuffers(1, &m_menuQuadVBO);
        glDeleteVertexArrays(1, &m_shapeVAO);
        glDeleteBuffers(1, &m_shapeVBO);
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

    void UIRenderer::RenderDropdown(UIDropdown const *dropdown) const {
        if (!dropdown->isVisible)
            return;
        if (dropdown->isOpened) {
            for (size_t i = 0; i < dropdown->entries.size(); i++)
            {
                RenderResource(dropdown->entryResource, static_cast<GLint>(dropdown->layer), dropdown->location.x + (dropdown->resource.width * dropdown->scale), dropdown->location.y - (dropdown->entryResource.height * dropdown->scale * (float) i), dropdown->scale);
                RenderText(dropdown->entries[i], dropdown->fontName, static_cast<GLint>(dropdown->layer), dropdown->location.x + dropdown->textOffset.x + (dropdown->resource.width * dropdown->scale),
                        dropdown->location.y + dropdown->textOffset.y - (dropdown->entryResource.height * dropdown->scale) * i, dropdown->scale, dropdown->entryTextColour[i], true);
            }
        }
        RenderResource(dropdown->resource, static_cast<GLint>(dropdown->layer), dropdown->location.x, dropdown->location.y, dropdown->scale);
        RenderText(dropdown->selectedEntry == -1 ? dropdown->text : dropdown->entries[dropdown->selectedEntry], dropdown->fontName, static_cast<GLint>(dropdown->layer), dropdown->location.x + dropdown->textOffset.x,
                   dropdown->location.y + dropdown->textOffset.y, dropdown->scale, dropdown->textColour, true);
    }

    void UIRenderer::RenderTextField(UITextField const *textField) const {
        RenderText(textField->text, textField->fontName, static_cast<GLint>(textField->layer), textField->location.x, textField->location.y,
                   textField->scale, textField->textColour, false);
    }

    void UIRenderer::RenderImage(UIImage const *image) const {
        RenderResource(image->resource, static_cast<GLint>(image->layer), image->location.x, image->location.y, image->scale);
    }

    void UIRenderer::RenderShape(UIShape const *shape) const {
        CHECK_F(shape->layer <= 200, "Layer: %d is outside of range 0-200", shape->layer);

        float const x = shape->location.x;
        float const y = shape->location.y;
        float const w = shape->width * shape->scale;
        float const h = shape->height * shape->scale;

        std::vector<GLfloat> vertices;

        switch (shape->shapeType) {
        case ShapeType::Rectangle:
            vertices = {
                x,     y + h, 0.f, 0.f,
                x,     y,     0.f, 0.f,
                x + w, y,     0.f, 0.f,
                x,     y + h, 0.f, 0.f,
                x + w, y,     0.f, 0.f,
                x + w, y + h, 0.f, 0.f,
            };
            break;

        case ShapeType::Triangle: {
            // Isosceles triangle: apex at top-center, base at bottom
            float const cx = x + w * 0.5f;
            vertices = {
                cx,    y + h, 0.f, 0.f, // apex
                x,     y,     0.f, 0.f, // bottom-left
                x + w, y,     0.f, 0.f, // bottom-right
            };
            break;
        }

        case ShapeType::Circle: {
            // Triangle fan: width is treated as the diameter, height is ignored
            float const cx = x + w * 0.5f;
            float const cy = y + w * 0.5f;
            float const r  = w * 0.5f;
            constexpr int segments  = 36;
            vertices.reserve(segments * 3 * 4);
            for (int i = 0; i < segments; ++i) {
                float const a0 = static_cast<float>(i) / segments * glm::two_pi<float>();
                float const a1 = static_cast<float>(i + 1) / segments * glm::two_pi<float>();
                // center
                vertices.insert(vertices.end(), {cx, cy, 0.f, 0.f});
                // edge vertex 0
                vertices.insert(vertices.end(), {cx + r * std::cos(a0), cy + r * std::sin(a0), 0.f, 0.f});
                // edge vertex 1
                vertices.insert(vertices.end(), {cx + r * std::cos(a1), cy + r * std::sin(a1), 0.f, 0.f});
            }
            break;
        }
        }

        m_uiShapeShader.use();
        m_uiShapeShader.loadLayer(static_cast<GLint>(shape->layer));
        m_uiShapeShader.loadColour(shape->colour);
        m_uiShapeShader.loadProjectionMatrix(m_projectionMatrix);

        glBindVertexArray(m_shapeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_shapeVBO);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(GLfloat)), vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 4));
        glBindVertexArray(0);

        m_uiShapeShader.unbind();
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
        UIFontAtlas const &atlas = it->second;

        // Activate the corresponding render state
        m_fontShader.use();
        m_fontShader.loadLayer(layer, isButtonText);
        m_fontShader.loadColour(colour);
        m_fontShader.loadProjectionMatrix(m_projectionMatrix);

        // Bind the atlas texture once
        m_fontShader.loadGlyphTexture(atlas.GetTextureID());

        glBindVertexArray(atlas.GetVAO());
        // Iterate through all characters

        for (int i = 0; i < text.length(); ++i) {
            // Calculate the code point of the current character
            uint32_t ch_idx = 0;
            if ((text[i] & 128)) {
                if ((text[i] & 64) && (text[i] & 32) && (text[i] & 16)) {
                    ch_idx = ((uint32_t)(text[i] & 0x0E)) << 18;
                    ch_idx += ((uint32_t)(text[i+1] & 0x3F)) << 12;
                    ch_idx += ((uint32_t)(text[i+2] & 0x3F)) << 6;
                    ch_idx += (uint32_t)(text[i+3] & 0x3F);
                    i += 3;
                } else if ((text[i] & 64) && (text[i] & 32)) {
                    ch_idx = ((uint32_t)(text[i] & 0x0F)) << 12;
                    ch_idx += ((uint32_t)(text[i+1] & 0x3F)) << 6;
                    ch_idx += (uint32_t)(text[i+2] & 0x3F);
                    i += 2;
                } else if ((text[i] & 64)) {
                    ch_idx = ((uint32_t)(text[i] & 0x1F)) << 6;
                    ch_idx += (uint32_t)(text[i+1] & 0x3F);
                    i += 1;
                } else {
                    // This is invalid
                    continue;
                }
            } else {
                // This is just an ascii character
                ch_idx = (uint32_t) text[i];
            }

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
} // namespace OpenNFS
