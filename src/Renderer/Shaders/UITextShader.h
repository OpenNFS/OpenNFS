#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>

namespace OpenNFS {
    class UITextShader : public BaseShader {
      public:
        explicit UITextShader();
        void loadLayer(GLint layer) const;
        void loadColour(glm::vec4 colour) const;
        void loadProjectionMatrix(glm::mat4 const &projection) const;
        void loadUITexture(GLuint textureID) const;
        void bindBuffer() const;
        void unbindBuffer() const;

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
