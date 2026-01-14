#pragma once

#include "../Camera/BaseCamera.h"
#include "Shaders/TextBillboardShader.h"
#include "UIRenderer.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace OpenNFS {
    class Text3DRenderer {
      public:
        explicit Text3DRenderer();
        ~Text3DRenderer();

        // Delete copy operations (we manage OpenGL resources)
        Text3DRenderer(Text3DRenderer const &) = delete;
        Text3DRenderer &operator=(Text3DRenderer const &) = delete;

        // Render 3D text at a world position
        void RenderText3D(std::string const &text, glm::vec3 const &worldPos, BaseCamera const &camera, float scale = 0.1f,
                          glm::vec4 const &colour = glm::vec4(1, 1, 1, 1)) const;

      private:
        TextBillboardShader m_textBillboardShader;
        UIFontAtlas m_fontAtlas;
        GLuint m_charQuadVAO, m_charQuadVBO;

        void InitQuadBuffers();
    };
} // namespace OpenNFS