#pragma once

#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <LinearMath/btDefaultMotionState.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <unordered_map>
#include <variant>

#include "Lights/BaseLight.h"
#include "Sound.h"
#include "Models/TrackModel.h"

#include "../Util/Utils.h"
#include "../Physics/AABB.h"
#include "../Physics/IAABB.h"
#include "../Physics/Car.h"

enum class EntityType {
    XOBJ,
    OBJ_POLY,
    LANE,
    SOUND,
    LIGHT,
    ROAD,
    GLOBAL,
    CAR,
    VROAD,
    VROAD_CEIL
};

const std::unordered_map<EntityType, std::string> entity_type_to_string {
  {EntityType::XOBJ, "XOBJ"},
  {EntityType::OBJ_POLY, "OBJ_POLY"},
  {EntityType::LANE, "LANE"},
  {EntityType::SOUND, "SOUND"},
  {EntityType::LIGHT, "LIGHT"},
  {EntityType::ROAD, "ROAD"},
  {EntityType::GLOBAL, "GLOBAL"},
  {EntityType::CAR, "CAR"},
  {EntityType::VROAD, "VROAD"},
  {EntityType::VROAD_CEIL, "VROAD_CEIL"}
};

static std::string get_string(EntityType type) {
    return entity_type_to_string.at(type);
}

typedef std::variant<TrackModel, std::shared_ptr<BaseLight>, Sound, Car*> EngineModel;

class Entity : public IAABB {
public:
    Entity(uint32_t parentTrackblockID,
           uint32_t entityID,
           NFSVersion nfsVersion,
           EntityType entityType,
           EngineModel glMesh = nullptr,
           uint32_t flags     = 0u,
           glm::vec3 fromA    = glm::vec3(0, 0, 0),
           glm::vec3 fromB    = glm::vec3(0, 0, 0),
           glm::vec3 toA      = glm::vec3(0, 0, 0),
           glm::vec3 toB      = glm::vec3(0, 0, 0));
    void Update(); // Update Entity position based on Physics engine
    AABB GetAABB() const;

    NFSVersion tag;
    EntityType type;
    EngineModel raw;
    btRigidBody* rigidBody;
    uint32_t parentTrackblockID, entityID;
    uint32_t flags;
    bool collideable = false;
    bool dynamic     = false;

    void _GenCollisionMesh();

private:
    glm::vec3 startPointA, startPointB, endPointA, endPointB;
    btTriangleMesh m_collisionMesh;
    btCollisionShape* m_collisionShape;
    btDefaultMotionState* m_motionState;
    AABB m_boundingBox;

    void _SetCollisionParameters();

    void _GenBoundingBox();
};
