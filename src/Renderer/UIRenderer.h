#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "../Shaders/FontShader.h"
#include "../Shaders/UIShader.h"
#include "../UI/UIElement.h"
#include "../Util/ImageLoader.h"

namespace OpenNFS {
    struct Character {
        GLuint textureID;   // ID handle of the glyph texture
        glm::ivec2 size;    // Size of glyph
        glm::ivec2 bearing; // Offset from baseline to left/top of glyph
        GLuint advance;     // Offset to advance to next glyph
    };

    class UIRenderer {
      public:
        explicit UIRenderer();
        ~UIRenderer();
        void BeginRenderPass();
        void EndRenderPass();

        // Per-UI Element render calls
        void RenderButton(UIButton *button);
        void RenderTextField(UITextField *textField);
        void RenderImage(UIImage *image);

      private:
        // Rendering Primitives
        void RenderText(std::string const &text, GLint layer, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 colour);
        void RenderResource(UIResource const &resource, GLint layer, GLfloat x, GLfloat y, GLfloat scale);
        void RenderResource(UIResource const &resource, GLint layer, GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat scale);

        // Maps to ease opengl display of character/menu resource
        std::map<GLchar, Character> m_characterMap;

        // OpenGL data
        GLuint m_fontQuadVAO, m_fontQuadVBO, m_menuQuadVAO, m_menuQuadVBO;
        glm::mat4 m_projectionMatrix{};
        // Render shaders
        FontShader m_fontShader;
        UIShader m_uiShader;
    };
} // namespace OpenNFS
