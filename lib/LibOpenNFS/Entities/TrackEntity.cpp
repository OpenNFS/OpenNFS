#include "TrackEntity.h"

namespace LibOpenNFS {
    TrackEntity::TrackEntity(uint32_t entityID, EntityType entityType, const Geometry &geometry, uint32_t flags) :
        type(entityType), geometry(geometry), flags(flags), entityID(entityID), hasGeometry(true) {
    }

    TrackEntity::TrackEntity(uint32_t entityID, EntityType entityType, uint32_t flags) : type(entityType), flags(flags), entityID(entityID) {
    }
} // namespace LibOpenNFS
