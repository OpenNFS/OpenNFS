#include "Entity.h"

#include "../Util/Utils.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h"
#include "Entities/TrackEntity.h"

#include <NFS3/NFS3Loader.h>

namespace OpenNFS {
    Entity::Entity(LibOpenNFS::TrackEntity &track_entity) : GLTrackModel(track_entity.geometry), trackEntity(&track_entity) {
        this->_GenCollisionMesh();
        this->_GenBoundingBox();
    }

    void Entity::_GenCollisionMesh() {
        switch (trackEntity->type) {
        case LibOpenNFS::EntityType::LANE:
            // Not collidable
            break;
        case LibOpenNFS::EntityType::LIGHT:
            m_collisionShape = std::make_unique<btBoxShape>(btVector3(5, 5, 5));
            break;
        case LibOpenNFS::EntityType::ROAD:
        case LibOpenNFS::EntityType::GLOBAL:
        case LibOpenNFS::EntityType::XOBJ:
        case LibOpenNFS::EntityType::OBJ_POLY: {
            if (trackEntity->dynamic) {
                // btBvhTriangleMeshShape doesn't collide when dynamic, use convex triangle mesh
                auto const mesh = std::make_unique<btTriangleMesh>();
                for (size_t vertIdx = 0; vertIdx < m_vertices.size() - 2; vertIdx += 3) {
                    glm::vec3 triangle0{m_vertices[vertIdx]};
                    glm::vec3 triangle1{m_vertices[vertIdx + 1]};
                    glm::vec3 triangle2{m_vertices[vertIdx + 2]};
                    mesh->addTriangle(Utils::glmToBullet(triangle0), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
                }
                m_collisionShape = std::make_unique<btConvexTriangleMeshShape>(mesh.get(), true);
            } else {
                // TODO: Use passable flags (flags&0x80) of VROAD to work out whether collidable
                for (size_t vertIdx = 0; vertIdx < m_vertices.size() - 2; vertIdx += 3) {
                    glm::vec3 triangle0{m_vertices[vertIdx]};
                    glm::vec3 triangle1{m_vertices[vertIdx + 1]};
                    glm::vec3 triangle2{m_vertices[vertIdx + 2]};
                    m_collisionMesh.addTriangle(Utils::glmToBullet(triangle0), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2),
                                                false);
                }
                m_collisionShape = std::make_unique<btBvhTriangleMeshShape>(&m_collisionMesh, true, true);
            }
        } break;
        default:
            CHECK_F(false, "Unable to generate a collision box for entity type: %s", magic_enum::enum_name(trackEntity->type).data());
            return;
        }

        float const entityMass{trackEntity->dynamic ? 100.f : 0.f};
        btVector3 localInertia;

        if (trackEntity->dynamic) {
            m_collisionShape->calculateLocalInertia(entityMass, localInertia);
        }

        m_motionState = std::make_unique<btDefaultMotionState>(
            Utils::MakeTransform(trackEntity->geometry.initialPosition, trackEntity->geometry.orientation));
        rigidBody = std::make_unique<btRigidBody>(
            btRigidBody::btRigidBodyConstructionInfo(entityMass, m_motionState.get(), m_collisionShape.get(), localInertia));
        rigidBody->setFriction(1.f);
        rigidBody->setUserPointer(this);
    }

    void Entity::_GenBoundingBox() {
        switch (trackEntity->type) {
        case LibOpenNFS::EntityType::XOBJ:
        case LibOpenNFS::EntityType::OBJ_POLY:
        case LibOpenNFS::EntityType::LANE:
        case LibOpenNFS::EntityType::ROAD:
        case LibOpenNFS::EntityType::GLOBAL: {
            auto [minVertex, maxVertex] = Utils::GenDimensions(m_vertices);
            m_boundingBox = AABB(minVertex, maxVertex, trackEntity->geometry.initialPosition);
            return;
        }
        case LibOpenNFS::EntityType::LIGHT: {
            auto const *baseLight = dynamic_cast<LibOpenNFS::BaseLight const *>(trackEntity);
            assert(baseLight);
            constexpr DimensionData meshDimensions{glm::vec3(-0.5, -0.5, -0.5), glm::vec3(0.5, 0.5, 0.5)};
            m_boundingBox = AABB(meshDimensions.minVertex, meshDimensions.maxVertex, baseLight->position);
            return;
        }
        default:
            CHECK_F(false, "Shouldn't be adding a %s entity to the AABB tree!", magic_enum::enum_name(trackEntity->type).data());
            break;
        }
    }

    void Entity::Update() {
        // We don't want to update Entities that aren't dynamic
        if (!trackEntity->dynamic || trackEntity->animData.empty()) {
            return;
        }

        btTransform trans;
        m_motionState->getWorldTransform(trans);

        if (!trackEntity->animData.empty()) {
            animKeyframeIndex = (animKeyframeIndex + 1) % trackEntity->animData.size();
            trackEntity->geometry.position =
                glm::vec3(trackEntity->animData.at(animKeyframeIndex).pt) * LibOpenNFS::NFS3::NFS3_SCALE_FACTOR;
        } else {
            trackEntity->geometry.position = Utils::bulletToGlm(trans.getOrigin());
            trackEntity->geometry.orientation = Utils::bulletToGlm(trans.getRotation());
        }

        UpdateMatrices();
    }

    AABB Entity::GetAABB() const {
        return m_boundingBox;
    }

    glm::vec3 Entity::GetDebugColour() const {
        switch (trackEntity->type) {
        case LibOpenNFS::EntityType::XOBJ:
            return glm::vec3(46, 204, 113) / 255.f;
        case LibOpenNFS::EntityType::OBJ_POLY:
            return glm::vec3(26, 188, 156) / 255.f;
        case LibOpenNFS::EntityType::LANE:
            return glm::vec3(44, 62, 80) / 255.f;
        case LibOpenNFS::EntityType::SOUND:
            return glm::vec3(231, 76, 60) / 255.f;
        case LibOpenNFS::EntityType::LIGHT:
            return glm::vec3(230, 126, 34) / 255.f;
        case LibOpenNFS::EntityType::ROAD:
            return glm::vec3(41, 128, 185) / 255.f;
        case LibOpenNFS::EntityType::GLOBAL:
            return glm::vec3(149, 165, 166) / 255.f;
        case LibOpenNFS::EntityType::CAR:
            return {0, 0, 0};
        case LibOpenNFS::EntityType::VROAD:
            return glm::vec3(243, 156, 18) / 255.f;
        }
        return {0, 0, 0};
    }

    LibOpenNFS::BaseLight *Entity::AsLight() const {
        return dynamic_cast<LibOpenNFS::BaseLight *>(trackEntity);
    }

    LibOpenNFS::EntityType Entity::Type() const {
        return trackEntity->type;
    }

    bool Entity::Collidable() const {
        return trackEntity->collidable;
    }

    bool Entity::Dynamic() const {
        return trackEntity->dynamic;
    }

    uint32_t Entity::RawFlags() const {
        return trackEntity->flags;
    }
} // namespace OpenNFS
