#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>

namespace OpenNFS {
    class UITextShader : public BaseShader {
      public:
        explicit UITextShader();
        void loadLayer(GLint layer);
        void loadColour(glm::vec4 colour);
        void loadProjectionMatrix(glm::mat4 const &projection);
        void loadUITexture(GLuint textureID);
        void bindBuffer();
        void unbindBuffer();

      protected:
        void bindAttributes() override;
        void getAllUniformLocations() override;
        void customCleanup() override;

        GLint projectionMatrixLocation;
        GLint attribute_coord;
        GLint layerLocation;
        GLint textureLocation;
        GLint colourLocation;
        GLuint vbo;
    };
} // namespace OpenNFS
