//
// Created by Amrik on 18/03/2019.
//

#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>

class MenuShader : public BaseShader {
public:
    explicit MenuShader();
    void loadColour(glm::vec3 colour);
    void loadProjectionMatrix(const glm::mat4 &projection);
    void loadGlyphTexture(GLuint textureID);
protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;

    GLint textGlyphSamplerLocation;
    GLint projectionMatrixLocation;
    GLint colourLocation;

    typedef BaseShader super;
};

