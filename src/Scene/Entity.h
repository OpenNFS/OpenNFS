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
    class Entity final : public IAABB, public GLTrackModel {
      public:
        explicit Entity(LibOpenNFS::TrackEntity &track_entity);
        ~Entity() override = default;
        void Update(); // Update Entity position based on Physics engine
        [[nodiscard]] AABB GetAABB() const override;
        [[nodiscard]] glm::vec3 GetDebugColour() const;
        [[nodiscard]] LibOpenNFS::BaseLight *AsLight() const;
        [[nodiscard]] LibOpenNFS::EntityType const Type() const;
        [[nodiscard]] bool Collidable() const;
        [[nodiscard]] bool Dynamic() const;
        [[nodiscard]] uint32_t RawFlags() const;

        std::unique_ptr<btRigidBody> rigidBody;

      private:
        btTriangleMesh m_collisionMesh;
        std::unique_ptr<btCollisionShape> m_collisionShape;
        std::unique_ptr<btDefaultMotionState> m_motionState;
        AABB m_boundingBox{};
        // Reference to the underlying track entity (which may be a BaseLight/TrackLight)
        LibOpenNFS::TrackEntity *trackEntity{nullptr};

        void _GenCollisionMesh();
        void _GenBoundingBox();

        size_t animKeyframeIndex{0};
    };
} // namespace OpenNFS
