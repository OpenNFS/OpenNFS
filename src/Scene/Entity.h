//
// Created by SYSTEM on 31/07/2018.
//

#pragma once

#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <LinearMath/btDefaultMotionState.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <boost/variant.hpp>

#include "../Util/Utils.h"
#include "Light.h"
#include "Sound.h"
#include "Track.h"

DEFINE_ENUM_WITH_STRING_CONVERSIONS(NFSVer, (UNKNOWN)(NFS_1)(NFS_2)(NFS_2_PS1)(NFS_2_SE)(NFS_3)(NFS_3_PS1)(NFS_4)(NFS_5));
DEFINE_ENUM_WITH_STRING_CONVERSIONS(EntityType, (XOBJ)(OBJ_POLY)(LANE)(SOUND)(LIGHT)(ROAD)(GLOBAL))
        
typedef boost::variant<Track, Light, Sound> EngineModel;

class Entity {
public:
    Entity(uint32_t parent_trackblock_id, uint32_t entity_id, NFSVer nfs_version, EntityType entity_type, EngineModel gl_mesh);
    void genPhysicsMesh();
    NFSVer tag;
    EntityType type;
    EngineModel glMesh;
    btRigidBody* rigidBody;
    uint32_t parentTrackblockID, entityID;

    private:
    btTriangleMesh physicsMesh;
    btCollisionShape* physicsShape;
    btDefaultMotionState* motionState;
};
