#pragma once

#include "BaseShader.h"
#include "../Scene/Models/LightModel.h"
#include "../Util/Utils.h"

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
