#include "UIFontAtlas.h"

#include "g3log/g3log.hpp"

namespace OpenNFS {
    UIFontAtlas::~UIFontAtlas() {
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

    UIFontAtlas::UIFontAtlas(UIFontAtlas &&other) noexcept
        : m_fontQuadVAO(other.m_fontQuadVAO), m_fontQuadVBO(other.m_fontQuadVBO), m_fontAtlasTexture(other.m_fontAtlasTexture),
          m_atlasWidth(other.m_atlasWidth), m_atlasHeight(other.m_atlasHeight), m_characters(other.m_characters) {
        // Transfer ownership - set other's handles to 0 so destructor won't delete them
        other.m_fontQuadVAO = 0;
        other.m_fontQuadVBO = 0;
        other.m_fontAtlasTexture = 0;
    }

    UIFontAtlas &UIFontAtlas::operator=(UIFontAtlas &&other) noexcept {
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

    bool UIFontAtlas::Initialise(FT_Library const &ft, std::string const &fontPath, int const size) {
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

    GLuint UIFontAtlas::GetTextureID() const {
        return m_fontAtlasTexture;
    }

    uint32_t UIFontAtlas::GetWidth() const {
        return m_atlasWidth;
    }

    uint32_t UIFontAtlas::GetHeight() const {
        return m_atlasHeight;
    }

    GLuint UIFontAtlas::GetVAO() const {
        return m_fontQuadVAO;
    }

    GLuint UIFontAtlas::GetVBO() const {
        return m_fontQuadVBO;
    }

    UIFontAtlas::CharacterInfo UIFontAtlas::GetCharacter(uint8_t const charIdx) const {
        return m_characters.at(charIdx);
    }
} // namespace OpenNFS