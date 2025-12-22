#pragma once

#include "BaseShader.h"

#include "../../Scene/GlobalLight.h"

namespace OpenNFS {
    class SkydomeShader : public BaseShader {
      public:
        SkydomeShader();

        void loadSunPosition(GlobalLight const *light) const;
        void loadMatrices(glm::mat4 const &projection, glm::mat4 const &view, glm::mat4 const &transformation) const;
        void loadStarRotationMatrix(glm::mat3 const &star_rotation_matrix) const;
        void loadTextures(GLuint clouds1TextureID, GLuint clouds2TextureID, GLuint sunTextureID, GLuint moonTextureID, GLuint tintTextureID,
                          GLuint tint2TextureID) const;
        void loadWeatherMixFactor(float weatherMixFactor) const;
        void loadTime(float time) const;

      protected:
        void bindAttributes() override;
        void getAllUniformLocations() override;
        void customCleanup() override;

        GLint transformationMatrixLocation;
        GLint projectionMatrixLocation;
        GLint viewMatrixLocation;
        GLint sunPositionLocation;
        GLint starRotationMatrixLocation;
        GLint tintTextureLocation;    // the color of the sky on the half-sphere where the sun is. (time x height)
        GLint tint2TextureLocation;   // the color of the sky on the opposite half-sphere. (time x height)
        GLint sunTextureLocation;     // sun texture (radius x time)
        GLint moonTextureLocation;    // moon texture (circular)
        GLint clouds1TextureLocation; // light clouds texture (spherical UV projection)
        GLint clouds2TextureLocation; // heavy clouds texture (spherical UV projection)
        GLint weatherLocation;        // mixing factor (0.5 to 1.0)
        GLint timeLocation;
    };
} // namespace OpenNFS
