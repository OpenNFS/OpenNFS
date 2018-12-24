//
// Created by Amrik Sadhra on 01/03/2018.
//

#pragma once

#include "BaseShader.h"
#include <sstream>
#include <glm/detail/type_mat4x4.hpp>
#include "../Config.h"
#include "../Util/Utils.h"
#include "../Scene/Model.h"
#include "../../include/TGALoader/TGALoader.h"
#include "../Scene/Light.h"
#include "../Physics/Car.h"

#define MAX_CAR_CONTRIB_LIGHTS 6

class CarShader : public BaseShader {
public:
    explicit CarShader(shared_ptr<Car> &current_car);
    void loadCarColor(glm::vec3 color);
    void loadCarTexture();
    void loadLights(std::vector<Light> lights);
    void loadSpecular(float damper, float reflectivity, float env_reflectivity);
    void loadProjectionViewMatrices(const glm::mat4 &projection, const glm::mat4 &view);
    void loadTransformationMatrix(const glm::mat4 &transformation);
    void bindTextureArray(GLuint textureArrayID);
    void setMultiTextured(bool multiTextured);
    void setPolyFlagged(bool polyFlagged);
    void load_tga_texture();
    void loadEnvironmentMapTexture();
protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;
    void loadEnvMapTextureData();
    shared_ptr<Car> car;
    GLint transformationMatrixLocation;
    GLint projectionMatrixLocation;
    GLint viewMatrixLocation;
    GLint envMapTextureLocation;
    GLint carTextureLocation;
    GLint colourLocation;
    GLint lightPositionLocation[MAX_CAR_CONTRIB_LIGHTS];
    GLint lightColourLocation[MAX_CAR_CONTRIB_LIGHTS];
    GLint attenuationLocation[MAX_CAR_CONTRIB_LIGHTS];
    GLint shineDamperLocation;
    GLint reflectivityLocation;
    GLint envReflectivityLocation;
    GLint carTextureArrayLocation; // Multitextured
    GLint isMultiTexturedLocation; // Multitextured
    GLint hasPolyFlagsLocation;    // PolyFlagged

    GLuint textureID = 0;
    GLuint envMapTextureID = 0;

    typedef BaseShader super;
};


