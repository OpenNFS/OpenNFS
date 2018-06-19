//
// Created by Amrik on 01/03/2018.
//

#pragma once

#include "BaseShader.h"
#include <sstream>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include "../Config.h"
#include "../Util/Utils.h"
#include "../Scene/Model.h"
#include "../../include/TGALoader/TGALoader.h"
#include "../Scene/Light.h"


class CarShader : public BaseShader {
public:
    CarShader(const std::string &car_name);
    void loadCarColor(glm::vec3 color);
    void loadCarTexture();
    void loadLight(Light light);
    void loadSpecular(float damper, float reflectivity, float env_reflectivity);
    void loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation);

protected:
    void load_tga_texture(const std::string &car_name);
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;
    GLint transformationMatrixLocation;
    GLint projectionMatrixLocation;
    GLint viewMatrixLocation;
    GLint envMapTextureLocation;
    GLint carTextureLocation;
    GLint colourLocation;
    GLint lightPositionLocation;
    GLint lightColourLocation;
    GLint shineDamperLocation;
    GLint reflectivityLocation;
    GLint envReflectivityLocation;
    GLuint textureID;
    GLuint envMapTextureID;

    typedef BaseShader super;

    void loadEnvironmentMapTexture();
    void LoadEnvMapTexture();
};


