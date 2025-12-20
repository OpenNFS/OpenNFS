#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>

namespace OpenNFS {
    class MiniMapShader : public BaseShader {
      public:
        explicit MiniMapShader();
        void loadColor(glm::vec3 color);
        void loadLayer(GLint layer);
        void loadProjectionMatrix(glm::mat4 const &projection);
        void loadTransformationMatrix(glm::mat4 const &transformation);

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
