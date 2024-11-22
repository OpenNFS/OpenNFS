#include "Entity.h"

#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h"
#include "Entities/BaseLight.h"
#include "../Util/Utils.h"

namespace OpenNFS {
    Entity::Entity(TrackEntity const &track_entity) : TrackEntity(track_entity),
                                                      GLTrackModel(geometry) {
        this->_GenCollisionMesh();
        this->_GenBoundingBox();
    }

    void Entity::_GenCollisionMesh() {
        auto center = glm::vec3(0, 0, 0);
        auto orientation = glm::quat(0, 0, 0, 1);

        switch (type) {
            case LibOpenNFS::EntityType::LIGHT:
            case LibOpenNFS::EntityType::LANE:
                // Not collideable
                break;
            case LibOpenNFS::EntityType::ROAD:
            case LibOpenNFS::EntityType::GLOBAL:
            case LibOpenNFS::EntityType::XOBJ:
            case LibOpenNFS::EntityType::OBJ_POLY: {
                const std::vector<glm::vec3> &vertices = m_vertices;
                center = initialPosition;
                if (dynamic) {
                    // btBvhTriangleMeshShape doesn't collide when dynamic, use convex triangle mesh
                    auto mesh = std::make_unique<btTriangleMesh>();
                    for (size_t vertIdx = 0; vertIdx < vertices.size() - 2; vertIdx += 3) {
                        glm::vec3 triangle = vertices[vertIdx];
                        glm::vec3 triangle1 = vertices[vertIdx + 1];
                        glm::vec3 triangle2 = vertices[vertIdx + 2];
                        mesh->addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1),
                                          Utils::glmToBullet(triangle2), false);
                    }
                    m_collisionShape = std::make_unique<btConvexTriangleMeshShape>(mesh.get(), true);
                } else {
                    // TODO: Use passable flags (flags&0x80) of VROAD to work out whether collidable
                    for (size_t vertIdx = 0; vertIdx < vertices.size() - 2; vertIdx += 3) {
                        glm::vec3 triangle = vertices[vertIdx];
                        glm::vec3 triangle1 = vertices[vertIdx + 1];
                        glm::vec3 triangle2 = vertices[vertIdx + 2];
                        m_collisionMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1),
                                                    Utils::glmToBullet(triangle2), false);
                    }
                    m_collisionShape = std::make_unique<btBvhTriangleMeshShape>(&m_collisionMesh, true, true);
                }
            }
            break;
            default:
                CHECK_F(false, "Unable to generate a collision box for entity type: %s", LibOpenNFS::get_string(type).c_str());
                return;
        }

        float const entityMass = dynamic ? 100.f : 0.f;
        btVector3 localInertia;

        if (dynamic) {
            m_collisionShape->calculateLocalInertia(entityMass, localInertia);
        }

        m_motionState = std::make_unique<btDefaultMotionState>(Utils::MakeTransform(center, orientation));
        rigidBody = std::make_unique<btRigidBody>(
            btRigidBody::btRigidBodyConstructionInfo(entityMass, m_motionState.get(), m_collisionShape.get(),
                                                     localInertia));
        rigidBody->setFriction(1.f);
        rigidBody->setUserPointer(this);
    }

    void Entity::_GenBoundingBox() {
        switch (type) {
            case LibOpenNFS::EntityType::XOBJ:
            case LibOpenNFS::EntityType::OBJ_POLY:
            case LibOpenNFS::EntityType::LANE:
            case LibOpenNFS::EntityType::ROAD:
            case LibOpenNFS::EntityType::GLOBAL: {
                auto [minVertex, maxVertex] = Utils::GenDimensions(m_vertices);
                m_boundingBox = AABB(minVertex + initialPosition,
                                     maxVertex + initialPosition, glm::vec3());
                return;
            }
            case LibOpenNFS::EntityType::LIGHT: {
                constexpr DimensionData meshDimensions{glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)};
                m_boundingBox = AABB(meshDimensions.minVertex, meshDimensions.maxVertex, position);
                return;
            }
            default:
                CHECK_F(false, "Shouldn't be adding a %s entity to the AABB tree!",
                        get_string(type).c_str());
                break;
        }
    }

    void Entity::Update() {
        // We don't want to update Entities that aren't dynamic
        if (!dynamic) {
            return;
        }
        btTransform trans;
        m_motionState->getWorldTransform(trans);
        position = Utils::bulletToGlm(trans.getOrigin());
        orientation = Utils::bulletToGlm(trans.getRotation());
        UpdateMatrices();
    }

    AABB Entity::GetAABB() const {
        switch (type) {
            case LibOpenNFS::EntityType::SOUND:
            case LibOpenNFS::EntityType::CAR:
            case LibOpenNFS::EntityType::VROAD:
                CHECK_F(false, "Shouldn't be adding a %s entity to the AABB tree!",
                        get_string(type).c_str());
            default:
                break;
        }

        return m_boundingBox;
    }
} // namespace OpenNFS
