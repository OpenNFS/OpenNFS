#pragma once

#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "../Renderer/Shaders/UITextShader.h"

namespace OpenNFS {
    class TextRenderer {
        /**
         * The atlas struct holds a texture that contains the visible US-ASCII characters
         * of a certain font rendered with a certain character height.
         * It also contains an array that contains all the information necessary to
         * generate the appropriate vertex and texture coordinates for each character.
         *
         * After the constructor is run, you don't need to use any FreeType functions anymore.
         */
        struct Atlas {
            GLuint tex; // texture object

            unsigned int w; // width of texture in pixels
            unsigned int h; // height of texture in pixels

            struct {
                float ax; // advance.x
                float ay; // advance.y

                float bw; // bitmap.width;
                float bh; // bitmap.height;

                float bl; // bitmap_left;
                float bt; // bitmap_top;

                float tx; // x offset of glyph in texture coordinates
                float ty; // y offset of glyph in texture coordinates
            } c[128];     // character information

            Atlas(FT_Face face, int height, UITextShader const &shader);
            ~Atlas();
        };

      public:
        TextRenderer();
        ~TextRenderer();
        void Render();

      private:
        void RenderText(char const *text, Atlas const *a, float x, float y, float sx, float sy) const;

        struct Point {
            GLfloat x;
            GLfloat y;
            GLfloat s;
            GLfloat t;
        };

        FT_Library ft;
        FT_Face face;

        constexpr static uint32_t MAX_WIDTH = 1024;

        Atlas *a48;
        Atlas *a24;
        Atlas *a12;

        UITextShader m_uiTextShader;
    };
} // namespace OpenNFS
