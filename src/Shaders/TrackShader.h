//
// Created by Amrik Sadhra on 01/03/2018.
//

#pragma once

#include "BaseShader.h"
#include "../Scene/Track.h"
#include "../Scene/Light.h"
#include <glm/detail/type_mat4x4.hpp>
#include <map>

#define MAX_LIGHTS 6

class TrackShader : public BaseShader {
public:
    TrackShader();
    void bindTextureArray(GLuint textureArrayID);
    void loadProjectionViewMatrices(const glm::mat4 &projection, const glm::mat4 &view); // These don't change between Shader binds, better to set state once for a track render pass
    void loadTransformMatrix(const glm::mat4 &transformation);
    void loadLightSpaceMatrix(const glm::mat4 &lightSpaceMatrix);
    void loadSpecular(float damper, float reflectivity);
    void loadLights(std::vector<Light> lights);
    void loadShadowMapTexture(GLuint shadowMapTextureID);
    void loadAmbientFactor(float ambientFactor);
    void setClassic(bool useClassic);

protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;
    GLint transformationMatrixLocation;
    GLint projectionMatrixLocation;
    GLint viewMatrixLocation;
    GLint lightSpaceMatrixLocation;
    GLint lightPositionLocation[MAX_LIGHTS];
    GLint lightColourLocation[MAX_LIGHTS];
    GLint attenuationLocation[MAX_LIGHTS];
    GLint shineDamperLocation;
    GLint reflectivityLocation;
    GLint useClassicLocation;
    GLint shadowMapTextureLocation;
    GLint ambientFactorLocation;
    GLint trackTextureArrayLocation;

    typedef BaseShader super;
};
