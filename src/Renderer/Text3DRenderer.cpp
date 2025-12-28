#include "Text3DRenderer.h"

namespace OpenNFS {
    Text3DRenderer::Text3DRenderer() {
        InitQuadBuffers();
        FT_Library ft;
        CHECK_F(!FT_Init_FreeType(&ft), "Failed to initialize FreeType library");
        m_fontAtlas.Initialise(ft, "../resources/ui/fonts/arial.ttf");
        FT_Done_FreeType(ft);
    }

    Text3DRenderer::~Text3DRenderer() {
        if (m_charQuadVAO != 0) {
            glDeleteVertexArrays(1, &m_charQuadVAO);
        }
        if (m_charQuadVBO != 0) {
            glDeleteBuffers(1, &m_charQuadVBO);
        }
    }

    void Text3DRenderer::InitQuadBuffers() {
        // Configure VAO/VBO for character quads
        glGenVertexArrays(1, &m_charQuadVAO);
        glGenBuffers(1, &m_charQuadVBO);
        glBindVertexArray(m_charQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_charQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);

        // Reset state
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void Text3DRenderer::RenderText3D(std::string const &text, glm::vec3 const &worldPos, BaseCamera const &camera, float const scale,
                                      glm::vec4 const &colour) const {
        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Optionally disable depth writing so text doesn't occlude itself or other objects
        // You can make this configurable if needed
        glDepthMask(GL_FALSE);

        // Activate shader and set uniforms
        m_textBillboardShader.use();
        m_textBillboardShader.loadMatrices(camera.projectionMatrix, camera.viewMatrix);
        m_textBillboardShader.loadColour(colour);
        m_textBillboardShader.loadGlyphTexture(m_fontAtlas.GetTextureID());

        glBindVertexArray(m_charQuadVAO);

        // Calculate text width for centering
        float totalWidth = 0;
        for (char c : text) {
            unsigned char const ch_idx = static_cast<unsigned char>(c);
            auto const &ch = m_fontAtlas.GetCharacter(ch_idx);
            totalWidth += ch.ax;
        }

        // Start position (centered)
        float xOffset = -totalWidth * scale * 0.5f;

        // Render each character
        for (char c : text) {
            unsigned char const ch_idx = static_cast<unsigned char>(c);
            auto const &ch = m_fontAtlas.GetCharacter(ch_idx);

            // Skip glyphs with no pixels
            if (ch.bw == 0 || ch.bh == 0) {
                xOffset += ch.ax;
                continue;
            }

            // Character dimensions
            float const w = ch.bw;
            float const h = ch.bh;
            float const xpos = xOffset + ch.bl;
            float const ypos = -(ch.bh - ch.bt);

            // Calculate texture coordinates from atlas
            float const tx = ch.tx;
            float const ty = ch.ty;
            float const tx2 = tx + ch.bw / static_cast<float>(m_fontAtlas.GetWidth());
            float const ty2 = ty + ch.bh / static_cast<float>(m_fontAtlas.GetHeight());

            // Build quad vertices (position in local billboard space + texture coords)
            GLfloat const vertices[6][4] = {{xpos, ypos + h, tx, ty}, {xpos, ypos, tx, ty2},      {xpos + w, ypos, tx2, ty2},
                                            {xpos, ypos + h, tx, ty}, {xpos + w, ypos, tx2, ty2}, {xpos + w, ypos + h, tx2, ty}};

            // Set billboard position for this character
            m_textBillboardShader.loadBillboardPos(worldPos);
            m_textBillboardShader.loadScale(scale);

            // Update VBO with character quad
            glBindBuffer(GL_ARRAY_BUFFER, m_charQuadVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Advance cursor for next glyph
            xOffset += ch.ax;
        }

        // Cleanup
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        m_textBillboardShader.unbind();

        // Restore depth mask
        glDepthMask(GL_TRUE);
    }
} // namespace OpenNFS