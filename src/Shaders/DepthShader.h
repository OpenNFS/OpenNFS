//
// Created by Amrik Sadhra on 16/10/2018.
//

#pragma once

#include "../Scene/Light.h"
#include "../Util/Utils.h"
#include "BaseShader.h"

class DepthShader : public BaseShader {
  public:
    DepthShader();
    void loadLightSpaceMatrix(const glm::mat4 &lightSpaceMatrix);
    void loadTransformMatrix(const glm::mat4 &transformationMatrix);
    void bindTextureArray(GLuint textureArrayID);

  protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;

    GLint lightSpaceMatrixLocation;
    GLint transformationMatrixLocation;
    GLint textureArrayLocation;

    typedef BaseShader super;
};
