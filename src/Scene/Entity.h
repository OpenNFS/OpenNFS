#pragma once

#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btDefaultMotionState.h>
#include <unordered_map>
#include <variant>
#include <Entities/TrackEntity.h>

#include "../Physics/AABB.h"
#include "../Physics/IAABB.h"
#include "../Renderer/Models/GLTrackModel.h"
#include "../Util/Utils.h"

namespace OpenNFS {
    class Entity : public IAABB {
    public:
        Entity(LibOpenNFS::TrackEntity *track_entity, GLModel *model);
        void Update(); // Update Entity position based on Physics engine
        AABB GetAABB() const;

        LibOpenNFS::TrackEntity *track_entity;
        GLModel *model;
        btRigidBody *rigid_body;
        uint32_t parent_trackblock_id;
        bool collideable = false;
        bool dynamic     = false;

        void _GenCollisionMesh();

    private:
        btTriangleMesh m_collisionMesh;
        btCollisionShape *m_collisionShape;
        btDefaultMotionState *m_motionState;
        AABB m_boundingBox;

        void _SetCollisionParameters();
        void _GenBoundingBox();
    };
} // namespace OpenNFS
