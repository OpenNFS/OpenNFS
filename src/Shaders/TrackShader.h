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
    void bindTrackTextures(const Track &track_block, map<unsigned int, GLuint> gl_id_map);
    void loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation);
    void loadSpecular(float damper, float reflectivity);
    void loadLights(std::vector<Light> lights);
    void setClassic(bool useClassic);

protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;
    GLint transformationMatrixLocation;
    GLint projectionMatrixLocation;
    GLint viewMatrixLocation;
    GLint lightPositionLocation[MAX_LIGHTS];
    GLint lightColourLocation[MAX_LIGHTS];
    GLint attenuationLocation[MAX_LIGHTS];
    GLint shineDamperLocation;
    GLint reflectivityLocation;
    GLint useClassicLocation;
    GLint TrackTexturesID;

    typedef BaseShader super;
};
