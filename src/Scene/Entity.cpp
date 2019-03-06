//
// Created by Amrik Sadhra on 31/07/2018.
//


#include "Entity.h"

Entity::Entity(uint32_t parent_trackblock_id, uint32_t entity_id, NFSVer nfs_version, EntityType entity_type,
               EngineModel gl_mesh, uint32_t flags) {
    tag = nfs_version;
    type = entity_type;
    glMesh = gl_mesh;
    this->flags = flags;
    parentTrackblockID = parent_trackblock_id;
    entityID = entity_id;
    setParameters();
}

Entity::Entity(uint32_t parent_trackblock_id, uint32_t entity_id, NFSVer nfs_version, EntityType entity_type, glm::vec3 from, glm::vec3 to) {
    ASSERT(entity_type == VROAD, "This constructor is purely for VROAD entities");
    tag = nfs_version;
    type = entity_type;
    parentTrackblockID = parent_trackblock_id;
    entityID = entity_id;
    startPoint = from;
    endPoint = to;
    setParameters();
}

void Entity::genPhysicsMesh() {
    if (type == LIGHT) {
        // Light mesh billboarded, generated AABB too large. Divide verts by scale factor to make smaller.
        std::vector<glm::vec3> vertices = boost::get<Light>(glMesh).m_vertices;
        glm::vec3 lightPosition = boost::get<Light>(glMesh).position;
        float lightBoundScaleF = 10.f;
        for (int i = 0; i < vertices.size() - 2; i += 3) {
            glm::vec3 triangle = glm::vec3((vertices[i].x / lightBoundScaleF) + lightPosition.x,
                                           (vertices[i].y / lightBoundScaleF) + lightPosition.y,
                                           (vertices[i].z / lightBoundScaleF) + lightPosition.z);
            glm::vec3 triangle1 = glm::vec3((vertices[i + 1].x / lightBoundScaleF) + lightPosition.x,
                                            (vertices[i + 1].y / lightBoundScaleF) + lightPosition.y,
                                            (vertices[i + 1].z / lightBoundScaleF) + lightPosition.z);
            glm::vec3 triangle2 = glm::vec3((vertices[i + 2].x / lightBoundScaleF) + lightPosition.x,
                                            (vertices[i + 2].y / lightBoundScaleF) + lightPosition.y,
                                            (vertices[i + 2].z / lightBoundScaleF) + lightPosition.z);
            physicsMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
        }
        physicsShape = new btBvhTriangleMeshShape(&physicsMesh, true, true);
    } else if (type == VROAD) {
        float wallHeight = 1.0f;
        auto *mesh = new btTriangleMesh();
        glm::vec3 triangle = glm::vec3(startPoint.x, startPoint.y - wallHeight, startPoint.z);
        glm::vec3 triangle1 = glm::vec3(startPoint.x, startPoint.y + wallHeight, startPoint.z);
        glm::vec3 triangle2 = glm::vec3(endPoint.x, endPoint.y + wallHeight, endPoint.z);
        mesh->addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
        glm::vec3 triangleA = glm::vec3(endPoint.x, endPoint.y - wallHeight, endPoint.z);
        glm::vec3 triangle1A = glm::vec3(endPoint.x, endPoint.y + wallHeight, endPoint.z);
        glm::vec3 triangle2A = glm::vec3(startPoint.x, startPoint.y - wallHeight, startPoint.z);
        mesh->addTriangle(Utils::glmToBullet(triangleA), Utils::glmToBullet(triangle1A), Utils::glmToBullet(triangle2A), false);
        physicsShape = new btConvexTriangleMeshShape(mesh);
    } else if (dynamic) {
        // btBvhTriangleMeshShape doesn't collide when dynamic, use convex triangle mesh
        auto *mesh = new btTriangleMesh();
        std::vector<glm::vec3> vertices = boost::get<Track>(glMesh).m_vertices;
        for (int i = 0; i < vertices.size() - 2; i += 3) {
            glm::vec3 triangle = glm::vec3(vertices[i].x, vertices[i].y, vertices[i].z);
            glm::vec3 triangle1 = glm::vec3(vertices[i + 1].x, vertices[i + 1].y, vertices[i + 1].z);
            glm::vec3 triangle2 = glm::vec3(vertices[i + 2].x, vertices[i + 2].y, vertices[i + 2].z);
            mesh->addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1),
                              Utils::glmToBullet(triangle2), false);
        }
        physicsShape = new btConvexTriangleMeshShape(mesh);
    } else {
        std::vector<glm::vec3> vertices = boost::get<Track>(glMesh).m_vertices;
        // TODO: Use passable flags (flags&0x80) of VROAD to work out whether collidable
        for (int i = 0; i < vertices.size() - 2; i += 3) {
            glm::vec3 triangle = glm::vec3(vertices[i].x, vertices[i].y, vertices[i].z);
            glm::vec3 triangle1 = glm::vec3(vertices[i + 1].x, vertices[i + 1].y, vertices[i + 1].z);
            glm::vec3 triangle2 = glm::vec3(vertices[i + 2].x, vertices[i + 2].y, vertices[i + 2].z);
            physicsMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
        }
        physicsShape = new btBvhTriangleMeshShape(&physicsMesh, true, true);
    }
    float entityMass = dynamic ? 100.f : 0.f;
    btVector3 localInertia;
    if (dynamic) {
        physicsShape->calculateLocalInertia(entityMass, localInertia);
    }
    motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    rigidBody = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(entityMass, motionState, physicsShape, localInertia));
    rigidBody->setFriction(btScalar(1.f));
    rigidBody->setUserPointer(this);
}

void Entity::update() {
    // We don't want to update Entities that aren't dynamic
    if (!((type == OBJ_POLY || type == XOBJ) && dynamic)) {
        return;
    }
    btTransform trans;
    motionState->getWorldTransform(trans);
    boost::get<Track>(glMesh).position = Utils::bulletToGlm(trans.getOrigin());
    boost::get<Track>(glMesh).orientation = Utils::bulletToGlm(trans.getRotation());
    boost::get<Track>(glMesh).update();
}

void Entity::setParameters() {
    switch (tag) {
        case NFS_3:
            switch (type) {
                case VROAD:
                    collideable = true;
                    dynamic = false;
                    break;
                case LIGHT:
                    collideable = false;
                    break;

                case SOUND:
                    collideable = false;
                    break;
                case ROAD:
                    collideable = true;
                    break;
                case OBJ_POLY:
                case XOBJ:
                    collideable = false;
                    break;
                    switch ((flags >> 4) & 0x7) {
                        case 1: // Hometown shack godray
                            collideable = false;
                            dynamic = false;
                            break;
                        case 3: // Hometown start fence
                            collideable = true;
                            dynamic = false;
                            break;
                        case 5: // Roadsign
                            collideable = true;
                            dynamic = true;
                            break;
                        case 6: // Hometown split marker
                            collideable = true;
                            dynamic = false;
                            break;
                        case 7:
                            collideable = true;
                            dynamic = false;
                            break;
                        default:
                            collideable = true;
                            dynamic = false;
                            break;
                    }
                    break;
            }
            break;
        default:
            collideable = true;
            dynamic = false;
            LOG(WARNING) << "Entity parameters are unset for " << ToString(tag);
    }
}


