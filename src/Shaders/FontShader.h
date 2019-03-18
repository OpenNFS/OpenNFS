//
// Created by Amrik on 17/03/2019.
//

#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>

class FontShader : public BaseShader {
public:
    explicit FontShader();
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

