#include "BaseLight.h"

namespace LibOpenNFS {
    BaseLight::BaseLight(uint32_t entityID, uint32_t flags, LightType type, glm::vec3 position, glm::vec4 colour) : TrackEntity(entityID, EntityType::LIGHT, flags) {
        this->type        = type;
        this->position    = position;
        this->colour      = colour;
        this->attenuation = glm::vec3(2.0f, 0.1f, 0.1f);
    }
} // namespace LibOpenNFS
