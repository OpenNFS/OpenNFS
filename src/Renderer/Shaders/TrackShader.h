#pragma once

#include "BaseShader.h"
#include <Entities/BaseLight.h>
#include <map>

#include "../../Scene/Spotlight.h"
#include "../../shaders/ShaderPreamble.h"

namespace OpenNFS {
    class TrackShader : public BaseShader {
      public:
        TrackShader();
        void bindTextureArray(GLuint textureArrayID) const;
        void loadProjectionViewMatrices(
            glm::mat4 const &projection,
            glm::mat4 const &view) const; // These don't change between Shader binds, better to set state once for a track render pass
        void loadTransformMatrix(glm::mat4 const &transformation) const;
        void loadLightSpaceMatrix(glm::mat4 const &lightSpaceMatrix) const;
        void loadSpecular(float damper, float reflectivity) const;
        void loadLights(std::vector<LibOpenNFS::BaseLight const *> const &lights) const;
        void loadSpotlight(Spotlight const &spotlight) const;
        void loadShadowMapTexture(GLuint shadowMapTextureID) const;
        void loadAmbientFactor(float ambientFactor) const;
        void setClassic(bool useClassic) const;

      protected:
        void bindAttributes() override;
        void getAllUniformLocations() override;
        void customCleanup() override;

        GLint transformationMatrixLocation;
        GLint projectionMatrixLocation;
        GLint viewMatrixLocation;
        GLint lightSpaceMatrixLocation;
        GLint lightPositionLocation[MAX_TRACK_CONTRIB_LIGHTS];
        GLint lightColourLocation[MAX_TRACK_CONTRIB_LIGHTS];
        GLint attenuationLocation[MAX_TRACK_CONTRIB_LIGHTS];
        GLint spotlightPositionLocation;
        GLint spotlightColourLocation;
        GLint spotlightDirectionLocation;
        GLint spotlightCutOffLocation;
        GLint shineDamperLocation;
        GLint reflectivityLocation;
        GLint useClassicLocation;
        GLint shadowMapTextureLocation;
        GLint ambientFactorLocation;
        GLint trackTextureArrayLocation;
    };
} // namespace OpenNFS
