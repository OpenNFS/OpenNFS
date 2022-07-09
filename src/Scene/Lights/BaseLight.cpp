#include "BaseLight.h"

BaseLight::BaseLight(LightType type, glm::vec3 position, glm::vec4 colour) {
    this->type        = type;
    this->position    = position;
    this->colour      = colour;
    this->attenuation = glm::vec3(2.0f, 0.1f, 0.1f);
}