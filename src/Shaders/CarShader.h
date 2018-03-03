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
    void loadMVPMatrix(glm::mat4 matrix);
    void loadCarColor(glm::vec3 color);
    void loadCarTexture();
    void loadLight(Light light);

    typedef BaseShader super;
protected:
    void load_tga_texture(const char *path);
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;
    GLint matrixLocation;
    GLint textureLocation;
    GLint colourLocation;
    GLint lightPositionLocation;
    GLint lightColourLocation;
    GLuint TextureID;
};


