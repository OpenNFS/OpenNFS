#pragma once

#include "BaseShader.h"
#include <Entities/BaseLight.h>
#include <map>

#include "../../Scene/GlobalLight.h"
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
        void loadCameraPosition(glm::vec3 const &cameraPosition) const;
        void loadTransformMatrix(glm::mat4 const &transformation) const;
        void loadSpecular(float damper, float reflectivity) const;
        void loadLights(std::vector<LibOpenNFS::BaseLight const *> const &lights) const;
        void loadSpotlights(std::vector<Spotlight> const &spotlights) const;
        void loadShadowMapTexture(GLuint shadowMapTextureID) const;
        void loadShadowMapTextureArray(GLuint shadowMapTextureArrayID) const;
        void loadCascadeData(CascadeData const &cascadeData) const;
        void loadAmbientFactor(float ambientFactor) const;
        void setClassic(bool useClassic) const;

      protected:
        void bindAttributes() override;
        void getAllUniformLocations() override;
        void customCleanup() override;

        GLint transformationMatrixLocation;
        GLint projectionMatrixLocation;
        GLint viewMatrixLocation;
        GLint lightPositionLocation[MAX_TRACK_CONTRIB_LIGHTS];
        GLint lightColourLocation[MAX_TRACK_CONTRIB_LIGHTS];
        GLint attenuationLocation[MAX_TRACK_CONTRIB_LIGHTS];
        GLint spotlightPositionLocation[MAX_SPOTLIGHTS];
        GLint spotlightColourLocation[MAX_SPOTLIGHTS];
        GLint spotlightDirectionLocation[MAX_SPOTLIGHTS];
        GLint spotlightInnerCutOffLocation[MAX_SPOTLIGHTS];
        GLint spotlightOuterCutOffLocation[MAX_SPOTLIGHTS];
        GLint shineDamperLocation;
        GLint reflectivityLocation;
        GLint useClassicLocation;
        GLint shadowMapTextureLocation;
        GLint shadowMapArrayLocation;
        GLint ambientFactorLocation;
        GLint cameraPositionLocation;
        GLint trackTextureArrayLocation;
        // CSM uniforms
        GLint lightSpaceMatricesLocation[CSM_NUM_CASCADES];
        GLint cascadePlaneDistancesLocation;
    };
} // namespace OpenNFS
