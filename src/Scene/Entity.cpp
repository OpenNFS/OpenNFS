#include "Entity.h"
#include "Entities/BaseLight.h"

namespace OpenNFS {
    Entity::Entity(LibOpenNFS::TrackEntity *track_entity) : track_entity(track_entity), model(std::make_unique<GLTrackModel>(&track_entity->geometry)) {
        this->_GenCollisionMesh();
        this->_GenBoundingBox();
    }

    void Entity::_GenCollisionMesh() {
        glm::vec3 center      = glm::vec3(0, 0, 0);
        glm::quat orientation = glm::quat(0, 0, 0, 1);

        switch (track_entity->type) {
        case LibOpenNFS::EntityType::LANE:
            break;
            //        case LibOpenNFS::EntityType::LIGHT: {
            //            std::shared_ptr<BaseLight> baseLight   = std::get<std::shared_ptr<BaseLight>>(raw);
            //            std::shared_ptr<TrackLight> trackLight = std::static_pointer_cast<TrackLight>(baseLight);
            //            // Light mesh billboarded, generated (Bullet) AABB too large. Divide verts by scale factor to make smaller.
            //            std::vector<glm::vec3> vertices = trackLight->model.m_vertices;
            //            center                          = baseLight->position;
            //            float lightBoundScaleF          = 10.f;
            //            for (int i = 0; i < vertices.size() - 2; i += 3) {
            //                glm::vec3 triangle  = glm::vec3((vertices[i].x / lightBoundScaleF), (vertices[i].y / lightBoundScaleF), (vertices[i].z / lightBoundScaleF));
            //                glm::vec3 triangle1 = glm::vec3((vertices[i + 1].x / lightBoundScaleF), (vertices[i + 1].y / lightBoundScaleF), (vertices[i + 1].z /
            //                lightBoundScaleF)); glm::vec3 triangle2 = glm::vec3((vertices[i + 2].x / lightBoundScaleF), (vertices[i + 2].y / lightBoundScaleF), (vertices[i + 2].z
            //                / lightBoundScaleF)); m_collisionMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
            //            }
            //            m_collisionShape = new btBvhTriangleMeshShape(&m_collisionMesh, true, true);
            //        } break;
        case LibOpenNFS::EntityType::ROAD:
        case LibOpenNFS::EntityType::GLOBAL:
        case LibOpenNFS::EntityType::XOBJ:
        case LibOpenNFS::EntityType::OBJ_POLY: {
            const std::vector<glm::vec3> &vertices = model->geometry->m_vertices;
            center                                 = model->geometry->initialPosition;
            orientation                            = model->geometry->orientation;
            if (track_entity->dynamic) {
                // btBvhTriangleMeshShape doesn't collide when dynamic, use convex triangle mesh
                auto mesh = std::make_unique<btTriangleMesh>();
                for (size_t vertIdx = 0; vertIdx < vertices.size() - 2; vertIdx += 3) {
                    glm::vec3 triangle  = vertices[vertIdx];
                    glm::vec3 triangle1 = vertices[vertIdx + 1];
                    glm::vec3 triangle2 = vertices[vertIdx + 2];
                    mesh->addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
                }
                m_collisionShape = std::make_unique<btConvexTriangleMeshShape>(mesh.get(), true);
            } else {
                // TODO: Use passable flags (flags&0x80) of VROAD to work out whether collidable
                for (size_t vertIdx = 0; vertIdx < vertices.size() - 2; vertIdx += 3) {
                    glm::vec3 triangle  = vertices[vertIdx];
                    glm::vec3 triangle1 = vertices[vertIdx + 1];
                    glm::vec3 triangle2 = vertices[vertIdx + 2];
                    m_collisionMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
                }
                m_collisionShape = std::make_unique<btBvhTriangleMeshShape>(&m_collisionMesh, true, true);
            }
        } break;
        default:
            CHECK_F(false, "Unable to generate a collision box for entity type: %s", LibOpenNFS::get_string(track_entity->type).c_str());
            return;
        }

        float const entityMass = track_entity->dynamic ? 100.f : 0.f;
        btVector3 localInertia;

        if (track_entity->dynamic) {
            m_collisionShape->calculateLocalInertia(entityMass, localInertia);
        }

        m_motionState = std::make_unique<btDefaultMotionState>(Utils::MakeTransform(center, orientation));
        rigidBody     = std::make_unique<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(entityMass, m_motionState.get(), m_collisionShape.get(), localInertia));
        rigidBody->setFriction(btScalar(1.f));
        rigidBody->setUserPointer(this);
    }

    void Entity::_GenBoundingBox() {
        switch (track_entity->type) {
        case LibOpenNFS::EntityType::XOBJ:
        case LibOpenNFS::EntityType::OBJ_POLY:
        case LibOpenNFS::EntityType::LANE:
        case LibOpenNFS::EntityType::ROAD:
        case LibOpenNFS::EntityType::GLOBAL: {
            DimensionData meshDimensions = Utils::GenDimensions(model->geometry->m_vertices);
            m_boundingBox = AABB(meshDimensions.minVertex + model->geometry->initialPosition, meshDimensions.maxVertex + model->geometry->initialPosition, glm::vec3());
            return;
        }
        case LibOpenNFS::EntityType::LIGHT: {
            // For now, only tracklights will have entities created
            auto *baseLight = static_cast<LibOpenNFS::BaseLight *>(track_entity);
            CHECK_F(baseLight->type == LibOpenNFS::LightType::TRACK_LIGHT, "Not ready to handle other light types at entity creation time");
            // std::shared_ptr<TrackLight> trackLight = std::static_pointer_cast<TrackLight>(baseLight);
            // DimensionData meshDimensions           = Utils::GenDimensions(trackLight->model.m_vertices);
            DimensionData meshDimensions{glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)};
            m_boundingBox = AABB(meshDimensions.minVertex, meshDimensions.maxVertex, baseLight->position);
            return;
        }
        default:
            CHECK_F(false, "Shouldn't be adding a %s entity to the AABB tree!", get_string(track_entity->type).c_str());
            break;
        }
    }

    void Entity::Update() {
        // We don't want to update Entities that aren't dynamic
        if (!track_entity->dynamic) {
            return;
        }
        btTransform trans;
        m_motionState->getWorldTransform(trans);
        model->geometry->position    = Utils::bulletToGlm(trans.getOrigin());
        model->geometry->orientation = Utils::bulletToGlm(trans.getRotation());
        model->update();
    }

    AABB Entity::GetAABB() const {
        switch (track_entity->type) {
        case LibOpenNFS::EntityType::SOUND:
        case LibOpenNFS::EntityType::CAR:
        case LibOpenNFS::EntityType::VROAD:
            CHECK_F(false, "Shouldn't be adding a %s entity to the AABB tree!", get_string(track_entity->type).c_str());
        default:
            break;
        }

        return m_boundingBox;
    }
} // namespace OpenNFS
