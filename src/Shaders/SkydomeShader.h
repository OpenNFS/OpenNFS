//
// Created by Amrik Sadhra on 21/08/2018.
//

#pragma once

#include "BaseShader.h"
#include "../Util/Utils.h"
#include "../Scene/Light.h"

class SkydomeShader : public BaseShader {
public:
    SkydomeShader();
    void loadSunPosition(const Light &sun);
    void loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation);
    void loadStarRotationMatrix(const glm::mat4 &star_rotation_matrix);
    void loadTextures(GLuint clouds1TextureID, GLuint clouds2TextureID, GLuint sunTextureID, GLuint moonTextureID, GLuint tintTextureID, GLuint tint2TextureID);
    void loadWeatherMixFactor(float weatherMixFactor);
    void loadTime(float time);

protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;

    GLint transformationMatrixLocation;
    GLint projectionMatrixLocation;
    GLint viewMatrixLocation;
    GLint sunPositionLocation;
    GLint starRotationMatrixLocation;
    GLint tintTextureLocation;//the color of the sky on the half-sphere where the sun is. (time x height)
    GLint tint2TextureLocation;//the color of the sky on the opposite half-sphere. (time x height)
    GLint sunTextureLocation;//sun texture (radius x time)
    GLint moonTextureLocation;//moon texture (circular)
    GLint clouds1TextureLocation;//light clouds texture (spherical UV projection)
    GLint clouds2TextureLocation;//heavy clouds texture (spherical UV projection)
    GLint weatherLocation;//mixing factor (0.5 to 1.0)
    GLint timeLocation;

    typedef BaseShader super;

};
