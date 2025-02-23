#pragma once

#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btDefaultMotionState.h>
#include <memory>

#include "../Physics/AABB.h"
#include "../Physics/IAABB.h"
#include "../Renderer/Models/GLTrackModel.h"
#include "Entities/BaseLight.h"
#include "Entities/TrackEntity.h"

namespace OpenNFS {
    class Entity final : public LibOpenNFS::TrackEntity, public IAABB, public GLTrackModel {
      public:
        explicit Entity(TrackEntity &track_entity);
        ~Entity() override = default;
        void Update(); // Update Entity position based on Physics engine
        AABB GetAABB() const override;
        glm::vec3 GetDebugColour() const;

        std::unique_ptr<btRigidBody> rigidBody;

        // TODO: Temp hackedy hack hack. Should be able to cast 'this' to BaseLight as it derives from TrackEntity...
        LibOpenNFS::BaseLight *baseLight{nullptr};
        explicit operator LibOpenNFS::BaseLight const *() const {
            return baseLight;
        }

      private:
        btTriangleMesh m_collisionMesh;
        std::unique_ptr<btCollisionShape> m_collisionShape;
        std::unique_ptr<btDefaultMotionState> m_motionState;
        AABB m_boundingBox{};

        void _GenCollisionMesh();
        void _GenBoundingBox();

        size_t animKeyframeIndex{0};
    };
} // namespace OpenNFS
