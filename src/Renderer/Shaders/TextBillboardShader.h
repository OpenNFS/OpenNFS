#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>

namespace OpenNFS {
    class TextBillboardShader : public BaseShader {
      public:
        explicit TextBillboardShader();
        void loadMatrices(glm::mat4 const &projection, glm::mat4 const &view) const;
        void loadBillboardPos(glm::vec3 const &pos) const;
        void loadScale(float scale) const;
        void loadColour(glm::vec4 const &colour) const;
        void loadGlyphTexture(GLuint textureID) const;

      protected:
        void bindAttributes() override;
        void getAllUniformLocations() override;
        void customCleanup() override;

        GLint projectionMatrixLocation;
        GLint viewMatrixLocation;
        GLint billboardPosLocation;
        GLint scaleLocation;
        GLint colourLocation;
        GLint textGlyphSamplerLocation;
    };
} // namespace OpenNFS