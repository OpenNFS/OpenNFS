#include "Entity.h"

#include "../Util/Utils.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h"
#include "Entities/TrackEntity.h"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/string_cast.hpp"

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
        btVector3 localInertia(0, 0, 0);

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
        if (!trackEntity->dynamic && trackEntity->animData.empty()) {
            return;
        }

        btTransform trans;
        m_motionState->getWorldTransform(trans);

        if (!trackEntity->animData.empty()) {
            if (++animFrameCounter >= trackEntity->animDelay) {
                animFrameCounter = 0;
                animKeyframeIndex = (animKeyframeIndex + 1) % trackEntity->animData.size();
            }

            // Get current and next keyframes for interpolation
            auto const &[pt, od1, od2, od3, od4]{trackEntity->animData.at(animKeyframeIndex)};
            size_t const nextKeyframeIndex = (animKeyframeIndex + 1) % trackEntity->animData.size();
            auto const &[ptNext, od1Next, od2Next, od3Next, od4Next]{trackEntity->animData.at(nextKeyframeIndex)};

            // Calculate interpolation factor (0.0 to 1.0)
            float const t =
                trackEntity->animDelay > 0 ? static_cast<float>(animFrameCounter) / static_cast<float>(trackEntity->animDelay) : 0.f;

            // Interpolate position (lerp)
            glm::vec3 const posStart = LibOpenNFS::Utils::FixedToFloat(pt) * LibOpenNFS::NFS3::NFS3_SCALE_FACTOR;
            glm::vec3 const posEnd = LibOpenNFS::Utils::FixedToFloat(ptNext) * LibOpenNFS::NFS3::NFS3_SCALE_FACTOR;
            position = glm::mix(posStart, posEnd, t);

            // Interpolate orientation (slerp)
            glm::quat const orientStart = glm::normalize(glm::quat(od1, od2, od3, od4));
            glm::quat const orientEnd = glm::normalize(glm::quat(od1Next, od2Next, od3Next, od4Next));
            // Apply -90 degree X rotation to convert from NFS3 coordinate system
            static glm::quat const xCorrection = glm::angleAxis(glm::radians(180.f), glm::vec3(1.f, 0.f, 1.f));
            orientation = xCorrection * glm::slerp(orientStart, orientEnd, t);
        } else {
            position = Utils::bulletToGlm(trans.getOrigin());
            orientation = Utils::bulletToGlm(trans.getRotation());
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

    bool Entity::Animated() const {
        return !trackEntity->animData.empty();
    }

    uint32_t Entity::RawFlags() const {
        return trackEntity->flags;
    }

    uint32_t Entity::ID() const {
        return trackEntity->entityID;
    }
} // namespace OpenNFS
