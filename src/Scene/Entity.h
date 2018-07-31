//
// Created by SYSTEM on 31/07/2018.
//

#pragma once

#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <LinearMath/btDefaultMotionState.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <boost/variant.hpp>
#include "Model.h"
#include "Light.h"
#include "Sound.h"
#include "Track.h"

enum NFSVer {
    UNKNOWN, NFS_1, NFS_2, NFS_2_PS1, NFS_2_SE, NFS_3, NFS_3_PS1, NFS_4, NFS_5
};

enum EntityType {
    XOBJ, OBJ_POLY, LANE, SOUND, LIGHT, ROAD
};

typedef boost::variant<shared_ptr<Track>, shared_ptr<Light>, shared_ptr<Sound>> EngineModel;

class Entity {
public:
    Entity(uint32_t parent_trackblock_id, uint32_t entity_id, NFSVer nfs_version, EntityType entity_type, EngineModel gl_mesh);
    void genPhysicsMesh();
    NFSVer tag;
    EntityType type;
    EngineModel glMesh;

    private:
    btTriangleMesh physicsMesh;
    btRigidBody* rigidBody;
    btCollisionShape* physicsShape;
    btDefaultMotionState* motionState;
};
