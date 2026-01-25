#pragma once

#include "BaseShader.h"

namespace OpenNFS {
    class SkidMarkShader : public BaseShader {
      public:
        SkidMarkShader();

        void loadProjectionViewMatrices(glm::mat4 const &projection, glm::mat4 const &view) const;
        void loadLifetime(float lifetime) const;
        void loadFadeStart(float fadeStart) const;
        void bindSkidTexture(GLuint textureID) const;

      protected:
        void bindAttributes() override;
        void getAllUniformLocations() override;
        void customCleanup() override;

      private:
        GLint m_projectionMatrixLocation;
        GLint m_viewMatrixLocation;
        GLint m_lifetimeLocation;
        GLint m_fadeStartLocation;
        GLint m_skidTextureLocation;
    };
} // namespace OpenNFS
