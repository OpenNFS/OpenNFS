#pragma once

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

enum LightType : uint8_t { TRACK_LIGHT, SPOT_LIGHT, GLOBAL_LIGHT };

class BaseLight {
public:
    BaseLight(LightType type, glm::vec3 position, glm::vec4 colour);
    LightType type;
    glm::vec3 position;
    glm::vec4 colour;
    glm::vec3 attenuation;
};
