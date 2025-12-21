#pragma once

#include "BaseShader.h"
#include <Entities/BaseLight.h>

#include "../../Physics/Car.h"
#include "../../Util/ImageLoader.h"
#include "../../shaders/ShaderPreamble.h"

namespace OpenNFS {
    class CarShader : public BaseShader {
      public:
        explicit CarShader();

        void loadCarColor(glm::vec4 color, glm::vec4 color_secondary = {0.0, 0.0, 0.0, 0.0}) const;

        void loadCarTexture(GLuint textureID) const;
        void loadLights(std::vector<LibOpenNFS::BaseLight const *> const &lights) const;
        void loadSpecular(float damper, float reflectivity, float env_reflectivity) const;
        void loadProjectionViewMatrices(glm::mat4 const &projection, glm::mat4 const &view) const;
        void loadTransformationMatrix(glm::mat4 const &transformation) const;
        void bindTextureArray(GLuint textureArrayID) const;
        void setMultiTextured(bool multiTextured) const;
        void setPolyFlagged(bool polyFlagged) const;
        void loadEnvironmentMapTexture() const;

      protected:
        void bindAttributes() override;
        void getAllUniformLocations() override;
        void customCleanup() override;
        void loadEnvMapTextureData();

        GLint transformationMatrixLocation;
        GLint projectionMatrixLocation;
        GLint viewMatrixLocation;
        GLint envMapTextureLocation;
        GLint carTextureLocation;
        GLint colourLocation;
        GLint colourSecondaryLocation;
        GLint lightPositionLocation[MAX_CAR_CONTRIB_LIGHTS];
        GLint lightColourLocation[MAX_CAR_CONTRIB_LIGHTS];
        GLint attenuationLocation[MAX_CAR_CONTRIB_LIGHTS];
        GLint shineDamperLocation;
        GLint reflectivityLocation;
        GLint envReflectivityLocation;
        GLint carTextureArrayLocation; // Multitextured
        GLint isMultiTexturedLocation; // Multitextured
        GLint hasPolyFlagsLocation;    // PolyFlagged

        GLuint envMapTextureID = 0;
    };
} // namespace OpenNFS
