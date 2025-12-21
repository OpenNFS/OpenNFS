#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>

namespace OpenNFS {
    class UIShader : public BaseShader {
      public:
        explicit UIShader();
        void loadLayer(GLint layer) const;
        void loadColour(glm::vec3 colour) const;
        void loadProjectionMatrix(glm::mat4 const &projection) const;
        void loadUITexture(GLuint textureID) const;

      protected:
        void bindAttributes() override;
        void getAllUniformLocations() override;
        void customCleanup() override;

        GLint menuTextureSamplerLocation;
        GLint projectionMatrixLocation;
        GLint layerLocation;
        GLint colourLocation;
    };
} // namespace OpenNFS
