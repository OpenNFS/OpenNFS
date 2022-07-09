#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>

class MenuShader : public BaseShader {
public:
    explicit MenuShader();
    void loadLayer(GLint layer);
    void loadColour(glm::vec3 colour);
    void loadProjectionMatrix(const glm::mat4 &projection);
    void loadMenuTexture(GLuint textureID);

protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;

    GLint menuTextureSamplerLocation;
    GLint projectionMatrixLocation;
    GLint layerLocation;
    GLint colourLocation;

    typedef BaseShader super;
};
