#pragma once

#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btDefaultMotionState.h>
#include <Entities/TrackEntity.h>

#include "../Physics/AABB.h"
#include "../Physics/IAABB.h"
#include "../Renderer/Models/GLTrackModel.h"

namespace OpenNFS {
    class Entity : public IAABB {
    public:
        explicit Entity(LibOpenNFS::TrackEntity *track_entity);
        void Update() const; // Update Entity position based on Physics engine
        AABB GetAABB() const override;

        LibOpenNFS::TrackEntity *track_entity;
        std::unique_ptr<GLModel> model;
        std::unique_ptr<btRigidBody> rigidBody;

    private:
        btTriangleMesh m_collisionMesh;
        std::unique_ptr<btCollisionShape> m_collisionShape;
        std::unique_ptr<btDefaultMotionState> m_motionState;
        AABB m_boundingBox;

        void _GenCollisionMesh();
        void _GenBoundingBox();
    };
} // namespace OpenNFS
