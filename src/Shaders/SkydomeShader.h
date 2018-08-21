//
// Created by Amrik Sadhra on 21/08/2018.
//

#pragma once

#include "BaseShader.h"
#include "../Util/Utils.h"
#include "../Scene/Light.h"

class SkydomeShader : public BaseShader {
public:
    SkydomeShader();
    void loadLight(Light board_light);
    void loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation);

protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;
    GLint transformationMatrixLocation;
    GLint projectionMatrixLocation;
    GLint viewMatrixLocation;
    GLint lightColourLocation;
    GLint billboardPosLocation;

    typedef BaseShader super;
};
