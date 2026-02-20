#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>
#include <glm/vec4.hpp>

namespace OpenNFS {
    class UIShapeShader : public BaseShader {
      public:
        explicit UIShapeShader();
        void loadLayer(GLint layer) const;
        void loadColour(glm::vec4 colour) const;
        void loadProjectionMatrix(glm::mat4 const &projection) const;

      protected:
        void bindAttributes() override;
        void getAllUniformLocations() override;
        void customCleanup() override;

        GLint projectionMatrixLocation;
        GLint layerLocation;
        GLint colourLocation;
    };
} // namespace OpenNFS