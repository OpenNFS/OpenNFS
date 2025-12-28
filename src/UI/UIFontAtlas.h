#pragma once

#include "GL/glew.h"

#include <array>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace OpenNFS {
    class UIFontAtlas {
      public:
        struct CharacterInfo {
            float ax; // advance.x
            float ay; // advance.y
            float bw; // bitmap.width;
            float bh; // bitmap.height;
            float bl; // bitmap_left;
            float bt; // bitmap_top;
            float tx; // x offset of glyph in texture coordinates
            float ty; // y offset of glyph in texture coordinates
        };

        explicit UIFontAtlas() = default;
        ~UIFontAtlas();

        // Delete copy operations (we manage OpenGL resources)
        UIFontAtlas(UIFontAtlas const &) = delete;
        UIFontAtlas &operator=(UIFontAtlas const &) = delete;

        // Move operations
        UIFontAtlas(UIFontAtlas &&other) noexcept;
        UIFontAtlas &operator=(UIFontAtlas &&other) noexcept;

        // Interface
        bool Initialise(FT_Library const &ft, std::string const &fontPath, int size = 48);
        GLuint GetTextureID() const;
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        GLuint GetVAO() const;
        GLuint GetVBO() const;
        CharacterInfo GetCharacter(uint8_t charIdx) const;

      private:
        GLuint m_fontQuadVAO = 0, m_fontQuadVBO = 0;
        GLuint m_fontAtlasTexture = 0;
        unsigned int m_atlasWidth = 0;
        unsigned int m_atlasHeight = 0;
        std::array<CharacterInfo, 128> m_characters{};
        constexpr static uint32_t MAX_WIDTH = 1024;
    };
} // namespace OpenNFS