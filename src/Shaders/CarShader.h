//
// Created by Amrik on 01/03/2018.
//

#pragma once

#include "BaseShader.h"
#include "../Scene/Model.h"
#include "../../include/TGALoader/TGALoader.h"
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>

class CarShader : public BaseShader {
public:
    CarShader();
    void loadMVPMatrix(glm::mat4 matrix);
    void loadCarColor(glm::vec3 color);
    void loadCarTexture();

    typedef BaseShader super;
protected:
    void load_tga_texture(const char *path);
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;
    GLint Matrix_UniformID;
    GLint Texture_UniformID;
    GLint Color_UniformID;
    GLuint TextureID;
};


