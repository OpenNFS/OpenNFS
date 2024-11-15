#include "TrackEntity.h"

#include <utility>
#include "Common/Utils.h"

namespace LibOpenNFS {
    TrackEntity::TrackEntity(uint32_t entityID, EntityType entityType, TrackGeometry geometry, uint32_t flags) :
        type(entityType), geometry(std::move(geometry)), flags(flags), entityID(entityID), hasGeometry(true) {
        this->_SetCollisionParameters();
    }

    TrackEntity::TrackEntity(uint32_t entityID, EntityType entityType, uint32_t flags) : type(entityType), flags(flags), entityID(entityID) {
        this->_SetCollisionParameters();
    }

    void TrackEntity::_SetCollisionParameters() {
        switch (type) {
        case EntityType::VROAD:
        case EntityType::LIGHT:
        case EntityType::SOUND:
        case EntityType::LANE:
        case EntityType::GLOBAL:
        case EntityType::CAR:
            collideable = false;
            dynamic     = false;
            break;
        case EntityType::ROAD:
            collideable = true;
            dynamic     = false;
            break;
        case EntityType::OBJ_POLY:
        case EntityType::XOBJ:
            collideable = true;
            dynamic     = false;
            /*switch ((flags >> 4) & 0x7) {
            case 1: // Hometown shack godray
                collideable = false;
                dynamic     = false;
                break;
            case 3: // Hometown start fence
                collideable = true;
                dynamic     = false;
                break;
            case 5: // Roadsign
                collideable = true;
                dynamic     = true;
                break;
            case 6: // Hometown split marker
                collideable = true;
                dynamic     = false;
                break;
            case 7:
                collideable = true;
                dynamic     = false;
                break;
            default:
                collideable = true;
                dynamic     = false;
                break;
            }*/
            break;
        default:
            collideable = false;
            dynamic     = false;
            // ASSERT(false, "Entity parameters are unset for %s", get_string(entityType).c_str());
            break;
        }
    }
} // namespace LibOpenNFS
