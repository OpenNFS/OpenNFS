//
// Created by Amrik Sadhra on 31/07/2018.
//

#pragma once

#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <LinearMath/btDefaultMotionState.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <boost/variant.hpp>

#include "../Enums.h"
#include "Light.h"
#include "Sound.h"
#include "Track.h"
#include "../Physics/Car.h"

typedef boost::variant<Track, Light, Sound, Car*> EngineModel;

class Entity {
public:
    Entity(uint32_t parent_trackblock_id, uint32_t entity_id, NFSVer nfs_version, EntityType entity_type, EngineModel gl_mesh, uint32_t flags);
    // TODO: Deprecate this Entity constructor, and use collidable flags from every NFS polygon type
    Entity(uint32_t parent_trackblock_id, uint32_t entity_id, NFSVer nfs_version, EntityType entity_type, EngineModel gl_mesh) : Entity(parent_trackblock_id, entity_id, nfs_version, entity_type, gl_mesh, 0u) {};
    // For physics engine only collision-meshes (not visible)
    Entity(uint32_t parent_trackblock_id, uint32_t entity_id, NFSVer nfs_version, EntityType entity_type, glm::vec3 from, glm::vec3 to);
    Entity(uint32_t parent_trackblock_id, uint32_t entity_id, NFSVer nfs_version, EntityType entity_type, glm::vec3 fromA, glm::vec3 fromB, glm::vec3 toA, glm::vec3 toB);
    void genPhysicsMesh();
    void update(); // Update Entity position based on Physics engine
    NFSVer tag;
    EntityType type;
    EngineModel glMesh;
    btRigidBody* rigidBody;
    uint32_t parentTrackblockID, entityID;
    uint32_t flags;
    bool collideable = false;
    bool dynamic = false;

    private:
    glm::vec3 startPointA, startPointB, endPointA, endPointB;
    btTriangleMesh physicsMesh;
    btCollisionShape* physicsShape;
    btDefaultMotionState* motionState;
    void setParameters();
};
