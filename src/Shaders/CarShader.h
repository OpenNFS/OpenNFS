//
// Created by Amrik on 01/03/2018.
//

#pragma once

#include "BaseShader.h"
#include "../Scene/Model.h"
#include "../../include/TGALoader/TGALoader.h"
#include "../Scene/Light.h"
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>

class CarShader : public BaseShader {
public:
    CarShader();
    void loadCarColor(glm::vec3 color);
    void loadCarTexture();
    void loadLight(Light light);
    void loadSpecular(float damper, float reflectivity);
    void loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation);

protected:
    void load_tga_texture(const char *path);
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
    GLuint textureID;
    GLuint envMapTextureID;

    typedef BaseShader super;

    void loadEnvironmentMapTexture();
    void LoadEnvMapTexture();
};


