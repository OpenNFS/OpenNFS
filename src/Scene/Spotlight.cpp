#include "Spotlight.h"

namespace OpenNFS {
    Spotlight::Spotlight() : BaseLight(0, 0, LibOpenNFS::LightType::SPOT_LIGHT, position, glm::vec4()) {
    }

    Spotlight::Spotlight(glm::vec3 const position, glm::vec3 const direction, glm::vec3 const colour, float const cutOff)
        : BaseLight(0, 0, LibOpenNFS::LightType::SPOT_LIGHT, position, glm::vec4(colour.r, colour.g, colour.b, 0)) {
        this->position = this->initialPosition = position;
        this->direction = direction;
        this->cutOff = cutOff;
    }
} // namespace OpenNFS
