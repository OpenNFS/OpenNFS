#pragma once

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

#include "Common/NFSVersion.h"
#include "Geometry.h"

namespace LibOpenNFS {
    enum class EntityType { XOBJ, OBJ_POLY, LANE, SOUND, LIGHT, ROAD, GLOBAL, CAR, VROAD };

    const std::unordered_map<EntityType, std::string> entity_type_to_string{{EntityType::XOBJ, "XOBJ"},     {EntityType::OBJ_POLY, "OBJ_POLY"}, {EntityType::LANE, "LANE"},
                                                                            {EntityType::SOUND, "SOUND"},   {EntityType::LIGHT, "LIGHT"},       {EntityType::ROAD, "ROAD"},
                                                                            {EntityType::GLOBAL, "GLOBAL"}, {EntityType::CAR, "CAR"},           {EntityType::VROAD, "VROAD"}};

    static std::string get_string(EntityType type) {
        return entity_type_to_string.at(type);
    }

    class TrackEntity {
    public:
        TrackEntity(uint32_t entityID, EntityType entityType, const Geometry &geometry, uint32_t flags = 0u);
        TrackEntity(uint32_t entityID, EntityType entityType, uint32_t flags = 0u);

        EntityType type;
        Geometry geometry;
        uint32_t entityID;
        uint32_t flags;
        bool hasGeometry{false};
    };
} // namespace LibOpenNFS