#pragma once

#include <GL/glew.h>
#include <ft2build.h>
#include <glm/glm.hpp>
#include FT_FREETYPE_H

#include "../UI/UIFont.h"
#include "../UI/UIResource.h"
#include "Shaders/UIShader.h"
#include "Shaders/UITextShader.h"

namespace OpenNFS {
    class UIButton;
    class UITextField;
    class UIImage;

    class UIRenderer {
      public:
        explicit UIRenderer();
        ~UIRenderer();

        // Interface
        void BeginRenderPass();
        static void EndRenderPass();

        // Font management
        bool GenerateAtlases(std::map<std::string, UIFont> const& fontMap);

        // Per-UI Element render calls
        void RenderButton(UIButton const *button) const;
        void RenderTextField(UITextField const *textField) const;
        void RenderImage(UIImage const *image) const;

      private:
        // Rendering Primitives
        void RenderText(std::string const &text, std::string const &fontName, GLint layer, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 colour) const;
        void RenderResource(UIResource const &resource, GLint layer, GLfloat x, GLfloat y, GLfloat scale) const;
        void RenderResource(UIResource const &resource, GLint layer, GLfloat x, GLfloat y, GLfloat width, GLfloat height,
                            GLfloat scale) const;

        class Atlas {
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

          public:
            explicit Atlas() = default;
            ~Atlas();

            // Delete copy operations (we manage OpenGL resources)
            Atlas(Atlas const &) = delete;
            Atlas &operator=(Atlas const &) = delete;

            // Move operations
            Atlas(Atlas &&other) noexcept;
            Atlas &operator=(Atlas &&other) noexcept;

            // Interface
            bool Initialise(FT_Library const &ft, std::string const &fontPath, int size = 48);
            GLuint GetTextureID() const;
            GLuint GetWidth() const;
            GLuint GetHeight() const;
            GLuint GetVAO() const;
            GLuint GetVBO() const;
            CharacterInfo GetCharacter(uint8_t charIdx) const;

          private:
            GLuint m_fontQuadVAO, m_fontQuadVBO;
            GLuint m_fontAtlasTexture;
            unsigned int m_atlasWidth;
            unsigned int m_atlasHeight;
            std::array<CharacterInfo, 128> m_characters;
            constexpr static uint32_t MAX_WIDTH = 1024;
        };

        std::map<std::string, Atlas> m_fontAtlases;
        GLuint m_menuQuadVAO, m_menuQuadVBO;
        glm::mat4 m_projectionMatrix{};

        // Render shaders
        UITextShader m_fontShader;
        UIShader m_uiShader;
    };
} // namespace OpenNFS
