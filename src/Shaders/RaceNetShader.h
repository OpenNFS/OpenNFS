//
// Created by Amrik Sadhra on 01/03/2018.
//

#pragma once

#include "BaseShader.h"

#include <glm/detail/type_mat4x4.hpp>

#define MAX_CAR_CONTRIB_LIGHTS 6

class RaceNetShader : public BaseShader {
  public:
    explicit RaceNetShader();
    void loadColor(glm::vec3 color);
    void loadProjectionMatrix(const glm::mat4 &projection);
    void loadTransformationMatrix(const glm::mat4 &transformation);

  protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;

    GLint transformationMatrixLocation;
    GLint projectionMatrixLocation;
    GLint colourLocation;

    typedef BaseShader super;
};
