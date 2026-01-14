#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>

namespace OpenNFS {
    class UITextShader : public BaseShader {
      public:
        explicit UITextShader();
        void loadLayer(GLint layer, bool isButtonText) const;
        void loadColour(glm::vec4 colour) const;
        void loadProjectionMatrix(glm::mat4 const &projection) const;
        void loadGlyphTexture(GLuint textureID) const;

      protected:
        void bindAttributes() override;
        void getAllUniformLocations() override;
        void customCleanup() override;

        GLint textGlyphSamplerLocation;
        GLint projectionMatrixLocation;
        GLint layerLocation;
        GLint colourLocation;
    };
} // namespace OpenNFS
