#pragma once

#include "Models/BaseLight.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

const float RADIANS_PER_TICK = 0.001f;

class GlobalLight : public BaseLight {
public:
    explicit GlobalLight(glm::vec3 lookAt, glm::vec3 position);
    void Update(float timeScaleFactor);
    void ChangeTarget(glm::vec3 lookAt);

    glm::mat4 lightSpaceMatrix;
    glm::mat4 viewMatrix;

private:
    glm::mat4 m_lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 160.f, 300.f);
    glm::vec3 m_lookAt;
};
