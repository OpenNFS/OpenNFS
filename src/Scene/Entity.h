#pragma once

#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btDefaultMotionState.h>
#include <unordered_map>
#include <variant>
#include <Models/TrackEntity.h>

#include "Models/BaseLight.h"
#include "Models/Sound.h"
#include "Models/TrackModel.h"

#include "../Physics/AABB.h"
#include "../Physics/Car.h"
#include "../Physics/IAABB.h"
#include "../Util/Utils.h"

typedef std::variant<TrackModel, std::shared_ptr<BaseLight>, Sound, Car *> EngineModel;

class Entity : public IAABB {
public:
    Entity(uint32_t parentTrackblockID, uint32_t entityID, NFSVersion nfsVersion, LibOpenNFS::EntityType entityType, EngineModel glMesh = nullptr, uint32_t flags = 0u,
           glm::vec3 fromA = glm::vec3(0, 0, 0), glm::vec3 fromB = glm::vec3(0, 0, 0), glm::vec3 toA = glm::vec3(0, 0, 0), glm::vec3 toB = glm::vec3(0, 0, 0));
    void Update(); // Update Entity position based on Physics engine
    AABB GetAABB() const;

    NFSVersion tag;
    LibOpenNFS::EntityType type;
    EngineModel raw;
    btRigidBody *rigidBody;
    uint32_t parentTrackblockID, entityID;
    uint32_t flags;
    bool collideable = false;
    bool dynamic     = false;

    void _GenCollisionMesh();

private:
    glm::vec3 startPointA, startPointB, endPointA, endPointB;
    btTriangleMesh m_collisionMesh;
    btCollisionShape *m_collisionShape;
    btDefaultMotionState *m_motionState;
    AABB m_boundingBox;

    void _SetCollisionParameters();
    void _GenBoundingBox();
};
