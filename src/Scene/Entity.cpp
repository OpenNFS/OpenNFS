#include "Entity.h"
#include "Entities/BaseLight.h"

namespace OpenNFS {
    Entity::Entity(LibOpenNFS::TrackEntity *track_entity, GLModel *model) : track_entity(track_entity), model(model) {
        this->_SetCollisionParameters();
        this->_GenCollisionMesh();
        this->_GenBoundingBox();
    }

    void Entity::_GenCollisionMesh() {
        glm::vec3 center      = glm::vec3(0, 0, 0);
        glm::quat orientation = glm::quat(0, 0, 0, 1);

        switch (track_entity.type) {
        default:
            return;
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
            auto *track_model                      = dynamic_cast<GLTrackModel *>(model);
            const std::vector<glm::vec3> &vertices = track_model->geometry->m_vertices;
            center                                 = track_model->geometry->initialPosition;
            orientation                            = track_model->geometry->orientation;
            if (dynamic) {
                // btBvhTriangleMeshShape doesn't collide when dynamic, use convex triangle mesh
                auto *mesh = new btTriangleMesh();
                for (size_t vertIdx = 0; vertIdx < vertices.size() - 2; vertIdx += 3) {
                    glm::vec3 triangle  = vertices[vertIdx];
                    glm::vec3 triangle1 = vertices[vertIdx + 1];
                    glm::vec3 triangle2 = vertices[vertIdx + 2];
                    mesh->addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
                }
                m_collisionShape = new btConvexTriangleMeshShape(mesh, true);
            } else {
                // TODO: Use passable flags (flags&0x80) of VROAD to work out whether collidable
                for (size_t vertIdx = 0; vertIdx < vertices.size() - 2; vertIdx += 3) {
                    glm::vec3 triangle  = vertices[vertIdx];
                    glm::vec3 triangle1 = vertices[vertIdx + 1];
                    glm::vec3 triangle2 = vertices[vertIdx + 2];
                    m_collisionMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
                }
                m_collisionShape = new btBvhTriangleMeshShape(&m_collisionMesh, true, true);
            }
        } break;
        }

        float const entityMass = dynamic ? 100.f : 0.f;
        btVector3 localInertia;

        if (dynamic) {
            m_collisionShape->calculateLocalInertia(entityMass, localInertia);
        }

        m_motionState = new btDefaultMotionState(Utils::MakeTransform(center, orientation));
        rigid_body    = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(entityMass, m_motionState, m_collisionShape, localInertia));
        rigid_body->setFriction(btScalar(1.f));
        rigid_body->setUserPointer(this);
    }

    void Entity::_GenBoundingBox() {
        switch (track_entity.type) {
        case LibOpenNFS::EntityType::XOBJ:
        case LibOpenNFS::EntityType::OBJ_POLY:
        case LibOpenNFS::EntityType::LANE:
        case LibOpenNFS::EntityType::ROAD:
        case LibOpenNFS::EntityType::GLOBAL: {
            auto *track_model            = dynamic_cast<GLTrackModel *>(model);
            DimensionData meshDimensions = Utils::GenDimensions(track_model->geometry->m_vertices);
            m_boundingBox                = AABB(meshDimensions.minVertex, meshDimensions.maxVertex, track_model->geometry->initialPosition);
            return;
        }
        case LibOpenNFS::EntityType::LIGHT: {
            // For now, only tracklights will have entities created
            auto *baseLight = static_cast<LibOpenNFS::BaseLight *>(track_entity);
            CHECK_F(baseLight->type == LibOpenNFS::LightType::TRACK_LIGHT, "Not ready to handle other light types at entity creation time");
            // std::shared_ptr<TrackLight> trackLight = std::static_pointer_cast<TrackLight>(baseLight);
            // DimensionData meshDimensions           = Utils::GenDimensions(trackLight->model.m_vertices);
            // m_boundingBox                          = AABB(meshDimensions.minVertex, meshDimensions.maxVertex, baseLight->position);
            return;
        }
        case LibOpenNFS::EntityType::SOUND:
        case LibOpenNFS::EntityType::CAR:
        case LibOpenNFS::EntityType::VROAD:
            return;
        default:
            CHECK_F(false, "Shouldn't be adding a %s entity to the AABB tree!", get_string(track_entity.type).c_str());
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
        auto *track_model                  = dynamic_cast<GLTrackModel *>(model);
        track_model->geometry->position    = Utils::bulletToGlm(trans.getOrigin());
        track_model->geometry->orientation = Utils::bulletToGlm(trans.getRotation());
        track_model->update();
    }

    void Entity::_SetCollisionParameters() {
        switch (track_entity.type) {
        case LibOpenNFS::EntityType::VROAD:
            collideable = true;
            dynamic     = false;
            break;
        case LibOpenNFS::EntityType::LIGHT:
            collideable = false;
        case LibOpenNFS::EntityType::SOUND:
            collideable = false;
            break;
        case LibOpenNFS::EntityType::ROAD:
            collideable = true;
            break;
        case LibOpenNFS::EntityType::OBJ_POLY:
        case LibOpenNFS::EntityType::XOBJ:
            collideable = false;
            break;
            switch ((flags >> 4) & 0x7) {
            case 1: // Hometown shack godray
                collideable = false;
                dynamic     = false;
                break;
            case 3: // Hometown start fence
                collideable = true;
                dynamic     = false;
                break;
            case 5: // Roadsign
                collideable = true;
                dynamic     = true;
                break;
            case 6: // Hometown split marker
                collideable = true;
                dynamic     = false;
                break;
            case 7:
                collideable = true;
                dynamic     = false;
                break;
            default:
                collideable = true;
                dynamic     = false;
                break;
            }
            break;
        case LibOpenNFS::EntityType::LANE:
        case LibOpenNFS::EntityType::GLOBAL:
        case LibOpenNFS::EntityType::CAR:
            break;
        default:
            collideable = true;
            dynamic     = false;
            LOG(WARNING) << "Entity parameters are unset for " << get_string(track_entity.type);
        }
    }

    AABB Entity::GetAABB() const {
        switch (track_entity.type) {
        case LibOpenNFS::EntityType::SOUND:
        case LibOpenNFS::EntityType::CAR:
        case LibOpenNFS::EntityType::VROAD:
            CHECK_F(false, "Shouldn't be adding a %s entity to the AABB tree!", get_string(track_entity.type).c_str());
        default:
            break;
        }

        return m_boundingBox;
    }
} // namespace OpenNFS
