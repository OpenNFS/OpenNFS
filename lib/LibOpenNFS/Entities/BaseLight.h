#pragma once

#include "TrackEntity.h"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace LibOpenNFS {
    enum LightType : uint8_t { TRACK_LIGHT, SPOT_LIGHT, GLOBAL_LIGHT };

    class BaseLight : public TrackEntity {
    public:
        BaseLight(uint32_t entityID, uint32_t flags, LightType type, glm::vec3 position, glm::vec4 colour);
        LightType type;
        glm::vec3 position;
        glm::vec4 colour;
        glm::vec3 attenuation;
    };
} // namespace LibOpenNFS
