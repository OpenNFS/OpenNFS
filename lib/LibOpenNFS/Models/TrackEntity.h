#pragma once

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

#include "Common/NFSVersion.h"
#include "Geometry.h"

namespace LibOpenNFS {
    enum class EntityType { XOBJ, OBJ_POLY, LANE, SOUND, LIGHT, ROAD, GLOBAL, CAR, VROAD, VROAD_CEIL };

    const std::unordered_map<EntityType, std::string> entity_type_to_string{
      {EntityType::XOBJ, "XOBJ"}, {EntityType::OBJ_POLY, "OBJ_POLY"}, {EntityType::LANE, "LANE"}, {EntityType::SOUND, "SOUND"}, {EntityType::LIGHT, "LIGHT"},
      {EntityType::ROAD, "ROAD"}, {EntityType::GLOBAL, "GLOBAL"},     {EntityType::CAR, "CAR"},   {EntityType::VROAD, "VROAD"}, {EntityType::VROAD_CEIL, "VROAD_CEIL"}};

    static std::string get_string(EntityType type) {
        return entity_type_to_string.at(type);
    }

    class TrackEntity {
    public:
        TrackEntity(uint32_t parentTrackblockID,
                    uint32_t entityID,
                    NFSVersion nfsVersion,
                    EntityType entityType,
                    const Geometry &geometry,
                    uint32_t flags  = 0u,
                    glm::vec3 fromA = glm::vec3(0, 0, 0),
                    glm::vec3 fromB = glm::vec3(0, 0, 0),
                    glm::vec3 toA   = glm::vec3(0, 0, 0),
                    glm::vec3 toB   = glm::vec3(0, 0, 0));

        NFSVersion tag;
        EntityType type;
        Geometry geometry;
        uint32_t parentTrackblockID, entityID;
        uint32_t flags;

    private:
        glm::vec3 startPointA, startPointB, endPointA, endPointB;
    };
} // namespace LibOpenNFS