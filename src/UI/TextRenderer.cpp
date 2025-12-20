#include "TextRenderer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <GL/glew.h>
#include <g3log/g3log.hpp>

namespace OpenNFS {
    /**
     * Render text using the currently loaded font and currently set font size.
     * Rendering starts at coordinates (x, y), z is always 0.
     * The pixel coordinates that the FreeType2 library uses are scaled by (sx, sy).
     */
    void TextRenderer::RenderText(char const *text, Atlas const *a, float x, float y, float const sx, float const sy) {
        /* Use the texture containing the atlas */
        glBindTexture(GL_TEXTURE_2D, a->tex);
        m_uiTextShader.loadUITexture(0);

        /* Set up the VBO for our vertex data */
        m_uiTextShader.bindBuffer();

        Point coords[6 * strlen(text)];
        int c = 0;

        /* Loop through all characters */
        for (auto p = reinterpret_cast<uint8_t const *>(text); *p; p++) {
            /* Calculate the vertex and texture coordinates */
            float const x2 = x + a->c[*p].bl * sx;
            float const y2 = -y - a->c[*p].bt * sy;
            float const w = a->c[*p].bw * sx;
            float const h = a->c[*p].bh * sy;

            /* Advance the cursor to the start of the next character */
            x += a->c[*p].ax * sx;
            y += a->c[*p].ay * sy;

            /* Skip glyphs that have no pixels */
            if (!w || !h)
                continue;

            coords[c++] = Point{x2, -y2, a->c[*p].tx, a->c[*p].ty};
            coords[c++] = Point{x2 + w, -y2, a->c[*p].tx + a->c[*p].bw / a->w, a->c[*p].ty};
            coords[c++] = Point{x2, -y2 - h, a->c[*p].tx, a->c[*p].ty + a->c[*p].bh / a->h};
            coords[c++] = Point{x2 + w, -y2, a->c[*p].tx + a->c[*p].bw / a->w, a->c[*p].ty};
            coords[c++] = Point{x2, -y2 - h, a->c[*p].tx, a->c[*p].ty + a->c[*p].bh / a->h};
            coords[c++] = Point{x2 + w, -y2 - h, a->c[*p].tx + a->c[*p].bw / a->w, a->c[*p].ty + a->c[*p].bh / a->h};
        }

        /* Draw all the character on the screen in one go */
        glBufferData(GL_ARRAY_BUFFER, sizeof coords, coords, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, c);

        m_uiTextShader.unbindBuffer();
    }

    TextRenderer::TextRenderer() {
        /* Initialize the FreeType2 library */
        CHECK_F(!FT_Init_FreeType(&ft), "Could not init freetype library");

        /* Load a font */
        CHECK_F(!FT_New_Face(ft, "../resources/ui/fonts/earth.ttf", 0, &face), "FREETYPE: Failed to load font");

        /* Create texture atlasses for several font sizes */
        a48 = new Atlas(face, 48, m_uiTextShader);
        a24 = new Atlas(face, 24, m_uiTextShader);
        a12 = new Atlas(face, 12, m_uiTextShader);
    }

    void TextRenderer::Render() {
        constexpr float sx = 2.0 / 2560.f;
        constexpr float sy = 2.0 / 1440.f;

        m_uiTextShader.use();

        /* White background */
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        /* Enable blending, necessary for our alpha texture */
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        constexpr glm::vec4 black{0, 0, 0, 1};
        constexpr glm::vec4 red{1, 0, 0, 1};
        constexpr glm::vec4 transparent_green{0, 1, 0, 0.5};

        /* Set color to black */
        m_uiTextShader.loadColour(black);

        /* Effects of alignment */
        RenderText("The Quick Brown Fox Jumps Over The Lazy Dog", a48, -1 + 8 * sx, 1 - 50 * sy, sx, sy);
        RenderText("The Misaligned Fox Jumps Over The Lazy Dog", a48, -1 + 8.5 * sx, 1 - 100.5 * sy, sx, sy);

        /* Scaling the texture versus changing the font size */
        RenderText("The Small Texture Scaled Fox Jumps Over The Lazy Dog", a48, -1 + 8 * sx, 1 - 175 * sy, sx * 0.5, sy * 0.5);
        RenderText("The Small Font Sized Fox Jumps Over The Lazy Dog", a24, -1 + 8 * sx, 1 - 200 * sy, sx, sy);
        RenderText("The Tiny Texture Scaled Fox Jumps Over The Lazy Dog", a48, -1 + 8 * sx, 1 - 235 * sy, sx * 0.25, sy * 0.25);
        RenderText("The Tiny Font Sized Fox Jumps Over The Lazy Dog", a12, -1 + 8 * sx, 1 - 250 * sy, sx, sy);

        /* Colors and transparency */
        RenderText("The Solid Black Fox Jumps Over The Lazy Dog", a48, -1 + 8 * sx, 1 - 430 * sy, sx, sy);

        m_uiTextShader.loadColour(red);
        RenderText("The Solid Red Fox Jumps Over The Lazy Dog", a48, -1 + 8 * sx, 1 - 330 * sy, sx, sy);
        RenderText("The Solid Red Fox Jumps Over The Lazy Dog", a48, -1 + 28 * sx, 1 - 450 * sy, sx, sy);

        m_uiTextShader.loadColour(transparent_green);
        RenderText("The Transparent Green Fox Jumps Over The Lazy Dog", a48, -1 + 8 * sx, 1 - 380 * sy, sx, sy);
        RenderText("The Transparent Green Fox Jumps Over The Lazy Dog", a48, -1 + 18 * sx, 1 - 440 * sy, sx, sy);
    }

    TextRenderer::Atlas::Atlas(FT_Face const face, int const height, UITextShader &shader) {
        FT_Set_Pixel_Sizes(face, 0, height);
        FT_GlyphSlot const g = face->glyph;

        unsigned int row_w = 0;
        unsigned int row_h = 0;
        w = 0;
        h = 0;

        memset(c, 0, sizeof c);

        /* Find minimum size for a texture holding all visible ASCII characters */
        for (int i = 32; i < 128; i++) {
            if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
                fprintf(stderr, "Loading character %c failed!\n", i);
                continue;
            }
            if (row_w + g->bitmap.width + 1 >= MAX_WIDTH) {
                w = std::max(w, row_w);
                h += row_h;
                row_w = 0;
                row_h = 0;
            }
            row_w += g->bitmap.width + 1;
            row_h = std::max(row_h, g->bitmap.rows);
        }

        w = std::max(w, row_w);
        h += row_h;

        /* Create a texture that will be used to hold all ASCII glyphs */
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        shader.loadUITexture(0);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, nullptr);

        /* We require 1 byte alignment when uploading texture data */
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        /* Clamping to edges is important to prevent artifacts when scaling */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        /* Linear filtering usually looks best for text */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        /* Paste all glyph bitmaps into the texture, remembering the offset */
        int ox = 0;
        int oy = 0;

        row_h = 0;

        for (int i = 32; i < 128; i++) {
            if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
                fprintf(stderr, "Loading character %c failed!\n", i);
                continue;
            }

            if (ox + g->bitmap.width + 1 >= MAX_WIDTH) {
                oy += row_h;
                row_h = 0;
                ox = 0;
            }

            glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);
            c[i].ax = g->advance.x >> 6;
            c[i].ay = g->advance.y >> 6;

            c[i].bw = g->bitmap.width;
            c[i].bh = g->bitmap.rows;

            c[i].bl = g->bitmap_left;
            c[i].bt = g->bitmap_top;

            c[i].tx = ox / static_cast<float>(w);
            c[i].ty = oy / static_cast<float>(h);

            row_h = std::max(row_h, g->bitmap.rows);
            ox += g->bitmap.width + 1;
        }

        fprintf(stderr, "Generated a %d x %d (%d kb) texture atlas\n", w, h, w * h / 1024);
    }

    TextRenderer::Atlas::~Atlas() {
        glDeleteTextures(1, &tex);
    }
} // namespace OpenNFS