#pragma once

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

#include "Common/NFSVersion.h"
#include "TrackGeometry.h"

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
        TrackEntity(uint32_t entityID, EntityType entityType, TrackGeometry geometry, uint32_t flags = 0u);
        TrackEntity(uint32_t entityID, EntityType entityType, uint32_t flags = 0u);

        EntityType type;
        TrackGeometry geometry;
        uint32_t entityID{0};
        uint32_t flags{0};
        bool hasGeometry{false};
        bool collideable{false};
        bool dynamic{false};

    private:
        void _SetCollisionParameters();
    };
} // namespace LibOpenNFS