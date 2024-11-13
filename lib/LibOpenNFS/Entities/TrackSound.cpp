#include "TrackSound.h"

namespace LibOpenNFS {
    TrackSound::TrackSound(uint32_t entityID, glm::vec3 position, uint32_t type) : TrackEntity(entityID, EntityType::SOUND, 0u) {
        this->position = position;
        this->type     = type;
    }
} // namespace LibOpenNFS