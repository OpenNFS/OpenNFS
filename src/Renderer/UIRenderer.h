#pragma once

#include <GL/glew.h>
#include <ft2build.h>
#include <glm/glm.hpp>
#include FT_FREETYPE_H

#include "../UI/UIFont.h"
#include "../UI/UIFontAtlas.h"
#include "../UI/UIResource.h"
#include "Shaders/UIShader.h"
#include "Shaders/UIShapeShader.h"
#include "Shaders/UITextShader.h"

namespace OpenNFS {
    class UIButton;
    class UIDropdown;
    class UITextField;
    class UIImage;
    class UIShape;

    class UIRenderer {
      public:
        explicit UIRenderer();
        ~UIRenderer();

        // Interface
        void BeginRenderPass();
        static void EndRenderPass();

        // Font management
        bool GenerateAtlases(std::map<std::string, UIFont> const &fontMap);

        // Per-UI Element render calls
        void RenderButton(UIButton const *button) const;
        void RenderDropdown(UIDropdown const *dropdown) const;
        void RenderTextField(UITextField const *textField) const;
        void RenderImage(UIImage const *image) const;
        void RenderShape(UIShape const *shape) const;

      private:
        // Rendering Primitives
        void RenderText(std::string const &text, std::string const &fontName, GLint layer, GLfloat x, GLfloat y, GLfloat scale,
                        glm::vec4 colour, bool isButtonText) const;
        void RenderResource(UIResource const &resource, GLint layer, GLfloat x, GLfloat y, GLfloat scale) const;
        void RenderResource(UIResource const &resource, GLint layer, GLfloat x, GLfloat y, GLfloat width, GLfloat height,
                            GLfloat scale) const;

        std::map<std::string, UIFontAtlas> m_fontAtlases;
        GLuint m_menuQuadVAO, m_menuQuadVBO;
        GLuint m_shapeVAO, m_shapeVBO;
        glm::mat4 m_projectionMatrix{};

        // Render shaders
        UITextShader m_fontShader;
        UIShader m_uiShader;
        UIShapeShader m_uiShapeShader;
    };
} // namespace OpenNFS
