#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>

namespace OpenNFS {
    class MiniMapShader : public BaseShader {
      public:
        explicit MiniMapShader();
        void loadColour(glm::vec3 colour) const;
        void loadLayer(GLint layer) const;
        void loadProjectionMatrix(glm::mat4 const &projection) const;
        void loadTransformationMatrix(glm::mat4 const &transformation) const;

      protected:
        void bindAttributes() override;
        void getAllUniformLocations() override;
        void customCleanup() override;

        GLint transformationMatrixLocation;
        GLint projectionMatrixLocation;
        GLint colourLocation;
        GLint layerLocation;
    };
} // namespace OpenNFS
