#pragma once

#include "../../Util/Utils.h"
#include "BaseShader.h"

namespace OpenNFS {
    class DepthShader : public BaseShader {
      public:
        DepthShader();

        void loadLightSpaceMatrix(glm::mat4 const &lightSpaceMatrix);

        void loadTransformMatrix(glm::mat4 const &transformationMatrix);

        void bindTextureArray(GLuint textureArrayID) const;

      protected:
        void bindAttributes() override;

        void getAllUniformLocations() override;

        void customCleanup() override;

        GLint lightSpaceMatrixLocation;
        GLint transformationMatrixLocation;
        GLint textureArrayLocation;
    };
} // namespace OpenNFS
