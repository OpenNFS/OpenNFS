#include "Entity.h"

Entity::Entity(uint32_t parentTrackblockID, uint32_t entityID, NFSVer nfsVersion, EntityType entityType,  EngineModel glMesh, uint32_t flags, glm::vec3 fromA, glm::vec3 fromB, glm::vec3 toA, glm::vec3 toB) {
    tag = nfsVersion;
    type = entityType;
    this->glMesh = glMesh;
    this->flags = flags;
    this->parentTrackblockID = parentTrackblockID;
    this->entityID = entityID;
    this->startPointA = fromA;
    this->startPointB = fromB;
    this->endPointA = toA;
    this->endPointB = toB;
    this->_SetCollisionParameters();
    this->_GenCollisionMesh();
}

void Entity::_GenCollisionMesh() {
    if(type == CAR || type == SOUND) {
        return;
    }
    else if (type == LIGHT)
    {
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
            m_collisionMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
        }
        m_collisionShape = new btBvhTriangleMeshShape(&m_collisionMesh, true, true);
    }
    else if (type == VROAD)
    {
        float wallHeight = 1.0f;
        auto *mesh = new btTriangleMesh();
        glm::vec3 triangle = glm::vec3(startPointA.x, startPointA.y - wallHeight, startPointA.z);
        glm::vec3 triangle1 = glm::vec3(startPointA.x, startPointA.y + wallHeight, startPointA.z);
        glm::vec3 triangle2 = glm::vec3(endPointA.x, endPointA.y + wallHeight, endPointA.z);
        mesh->addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
        glm::vec3 triangleA = glm::vec3(endPointA.x, endPointA.y - wallHeight, endPointA.z);
        glm::vec3 triangle1A = glm::vec3(endPointA.x, endPointA.y + wallHeight, endPointA.z);
        glm::vec3 triangle2A = glm::vec3(startPointA.x, startPointA.y - wallHeight, startPointA.z);
        mesh->addTriangle(Utils::glmToBullet(triangleA), Utils::glmToBullet(triangle1A), Utils::glmToBullet(triangle2A), false);
        m_collisionShape = new btConvexTriangleMeshShape(mesh);
    }
    else if (type == VROAD_CEIL)
    {
        float ceilHeight = 0.5f;
        auto *mesh = new btTriangleMesh();
        glm::vec3 triangle   = glm::vec3(startPointA.x, startPointA.y + ceilHeight, startPointA.z);
        glm::vec3 triangle2  = glm::vec3(startPointB.x, startPointB.y + ceilHeight, startPointB.z);
        glm::vec3 triangle1  = glm::vec3(endPointA.x,   endPointA.y   + ceilHeight, endPointA.z);
        mesh->addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
        glm::vec3 triangleA  = glm::vec3(endPointA.x,   endPointA.y   + ceilHeight, endPointA.z);
        glm::vec3 triangle1A = glm::vec3(endPointB.x,   endPointB.y   + ceilHeight, endPointB.z);
        glm::vec3 triangle2A = glm::vec3(startPointB.x, startPointB.y + ceilHeight, startPointB.z);
        mesh->addTriangle(Utils::glmToBullet(triangleA), Utils::glmToBullet(triangle1A), Utils::glmToBullet(triangle2A), false);
        m_collisionShape = new btConvexTriangleMeshShape(mesh);
    }
    else if (dynamic)
    {
        // btBvhTriangleMeshShape doesn't collide when dynamic, use convex triangle mesh
        auto *mesh = new btTriangleMesh();
        std::vector<glm::vec3> vertices = boost::get<Track>(glMesh).m_vertices;
        for (int i = 0; i < vertices.size() - 2; i += 3)
        {
            glm::vec3 triangle = glm::vec3(vertices[i].x, vertices[i].y, vertices[i].z);
            glm::vec3 triangle1 = glm::vec3(vertices[i + 1].x, vertices[i + 1].y, vertices[i + 1].z);
            glm::vec3 triangle2 = glm::vec3(vertices[i + 2].x, vertices[i + 2].y, vertices[i + 2].z);
            mesh->addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1),
                              Utils::glmToBullet(triangle2), false);
        }
        m_collisionShape = new btConvexTriangleMeshShape(mesh);
    }
    else
    {
        std::vector<glm::vec3> vertices = boost::get<Track>(glMesh).m_vertices;
        // TODO: Use passable flags (flags&0x80) of VROAD to work out whether collidable
        for (int i = 0; i < vertices.size() - 2; i += 3)
        {
            glm::vec3 triangle = glm::vec3(vertices[i].x, vertices[i].y, vertices[i].z);
            glm::vec3 triangle1 = glm::vec3(vertices[i + 1].x, vertices[i + 1].y, vertices[i + 1].z);
            glm::vec3 triangle2 = glm::vec3(vertices[i + 2].x, vertices[i + 2].y, vertices[i + 2].z);
            m_collisionMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
        }
        m_collisionShape = new btBvhTriangleMeshShape(&m_collisionMesh, true, true);
    }
    float entityMass = dynamic ? 100.f : 0.f;
    btVector3 localInertia;
    if (dynamic) {
        m_collisionShape->calculateLocalInertia(entityMass, localInertia);
    }
    m_motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    rigidBody = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(entityMass, m_motionState, m_collisionShape, localInertia));
    rigidBody->setFriction(btScalar(1.f));
    rigidBody->setUserPointer(this);
}

void Entity::Update() {
    // We don't want to update Entities that aren't dynamic
    if (!((type == OBJ_POLY || type == XOBJ) && dynamic)) {
        return;
    }
    btTransform trans;
    m_motionState->getWorldTransform(trans);
    boost::get<Track>(glMesh).position = Utils::bulletToGlm(trans.getOrigin());
    boost::get<Track>(glMesh).orientation = Utils::bulletToGlm(trans.getRotation());
    boost::get<Track>(glMesh).update();
}

void Entity::_SetCollisionParameters() {
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

AABB Entity::getAABB() const
{
    switch(type)
    {
        case XOBJ:
        case OBJ_POLY:
        case LANE:
        case ROAD:
        case LIGHT:
        {
            DimensionData meshDimensions = Utils::GenDimensions(boost::get<Track>(glMesh).m_vertices);
            return AABB(meshDimensions.bottomLeft.x, meshDimensions.bottomLeft.y, meshDimensions.bottomLeft.z, meshDimensions.topRight.x, meshDimensions.topRight.y, meshDimensions.topRight.z);
        }
        case SOUND:
        case GLOBAL:
        case CAR:
        case VROAD:
        case VROAD_CEIL:
        default:
            ASSERT(false, "Shouldn't be adding a " << ToString(type) << " entity to the AABB tree!");
            break;
    }
    return AABB();
}
