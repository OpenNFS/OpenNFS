#include "GlobalLight.h"

#include <glm/gtc/matrix_transform.hpp>

GlobalLight::GlobalLight() {
    glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 160.f, 300.f);
    lightSpaceMatrix = lightProjection * viewMatrix;
}
