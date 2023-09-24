#include "TrackEntity.h"

namespace LibOpenNFS {
    TrackEntity::TrackEntity(uint32_t parentTrackblockID,
                             uint32_t entityID,
                             NFSVersion nfsVersion,
                             EntityType entityType,
                             const Geometry &geometry,
                             uint32_t flags,
                             glm::vec3 fromA,
                             glm::vec3 fromB,
                             glm::vec3 toA,
                             glm::vec3 toB) :
        tag(nfsVersion),
        type(entityType),
        geometry(geometry),
        flags(flags),
        parentTrackblockID(parentTrackblockID),
        entityID(entityID),
        startPointA(fromA),
        startPointB(fromB),
        endPointA(toA),
        endPointB(toB) {
    }
} // namespace LibOpenNFS
