#include "Entity.h"
#include "Lights/TrackLight.h"

Entity::Entity(uint32_t parentTrackblockID,
               uint32_t entityID,
               NFSVer nfsVersion,
               EntityType entityType,
               EngineModel glMesh,
               uint32_t flags,
               glm::vec3 fromA,
               glm::vec3 fromB,
               glm::vec3 toA,
               glm::vec3 toB)
{
    tag                      = nfsVersion;
    type                     = entityType;
    this->raw                = glMesh;
    this->flags              = flags;
    this->parentTrackblockID = parentTrackblockID;
    this->entityID           = entityID;
    this->startPointA        = fromA;
    this->startPointB        = fromB;
    this->endPointA          = toA;
    this->endPointB          = toB;
    this->_SetCollisionParameters();
    // this->_GenCollisionMesh();
    this->_GenBoundingBox();
}

void Entity::_GenCollisionMesh()
{
    glm::vec3 center      = glm::vec3(0, 0, 0);
    glm::quat orientation = glm::quat(0, 0, 0, 1);

    switch (type)
    {
    case SOUND:
    case CAR:
    case LANE:
        return;
    case LIGHT:
    {
        std::shared_ptr<BaseLight> baseLight   = boost::get<std::shared_ptr<BaseLight>>(raw);
        std::shared_ptr<TrackLight> trackLight = std::static_pointer_cast<TrackLight>(baseLight);
        // Light mesh billboarded, generated (Bullet) AABB too large. Divide verts by scale factor to make smaller.
        std::vector<glm::vec3> vertices = trackLight->model.m_vertices;
        center                          = baseLight->position;
        float lightBoundScaleF          = 10.f;
        for (int i = 0; i < vertices.size() - 2; i += 3)
        {
            glm::vec3 triangle  = glm::vec3((vertices[i].x / lightBoundScaleF), (vertices[i].y / lightBoundScaleF), (vertices[i].z / lightBoundScaleF));
            glm::vec3 triangle1 = glm::vec3((vertices[i + 1].x / lightBoundScaleF), (vertices[i + 1].y / lightBoundScaleF), (vertices[i + 1].z / lightBoundScaleF));
            glm::vec3 triangle2 = glm::vec3((vertices[i + 2].x / lightBoundScaleF), (vertices[i + 2].y / lightBoundScaleF), (vertices[i + 2].z / lightBoundScaleF));
            m_collisionMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
        }
        m_collisionShape = new btBvhTriangleMeshShape(&m_collisionMesh, true, true);
    }
    break;
    case VROAD:
    {
        float wallHeight    = 1.0f;
        auto *mesh          = new btTriangleMesh();
        glm::vec3 triangle  = glm::vec3(startPointA.x, startPointA.y - wallHeight, startPointA.z);
        glm::vec3 triangle1 = glm::vec3(startPointA.x, startPointA.y + wallHeight, startPointA.z);
        glm::vec3 triangle2 = glm::vec3(endPointA.x, endPointA.y + wallHeight, endPointA.z);
        mesh->addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);

        glm::vec3 triangleA  = glm::vec3(endPointA.x, endPointA.y - wallHeight, endPointA.z);
        glm::vec3 triangle1A = glm::vec3(endPointA.x, endPointA.y + wallHeight, endPointA.z);
        glm::vec3 triangle2A = glm::vec3(startPointA.x, startPointA.y - wallHeight, startPointA.z);
        mesh->addTriangle(Utils::glmToBullet(triangleA), Utils::glmToBullet(triangle1A), Utils::glmToBullet(triangle2A), false);
        m_collisionShape = new btConvexTriangleMeshShape(mesh);
    }
    break;
    case VROAD_CEIL:
    {
        float ceilHeight    = 0.5f;
        auto *mesh          = new btTriangleMesh();
        glm::vec3 triangle  = glm::vec3(startPointA.x, startPointA.y + ceilHeight, startPointA.z);
        glm::vec3 triangle2 = glm::vec3(startPointB.x, startPointB.y + ceilHeight, startPointB.z);
        glm::vec3 triangle1 = glm::vec3(endPointA.x, endPointA.y + ceilHeight, endPointA.z);
        mesh->addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);

        glm::vec3 triangleA  = glm::vec3(endPointA.x, endPointA.y + ceilHeight, endPointA.z);
        glm::vec3 triangle1A = glm::vec3(endPointB.x, endPointB.y + ceilHeight, endPointB.z);
        glm::vec3 triangle2A = glm::vec3(startPointB.x, startPointB.y + ceilHeight, startPointB.z);
        mesh->addTriangle(Utils::glmToBullet(triangleA), Utils::glmToBullet(triangle1A), Utils::glmToBullet(triangle2A), false);
        m_collisionShape = new btConvexTriangleMeshShape(mesh);
    }
    break;
    case ROAD:
    case GLOBAL:
    case XOBJ:
    case OBJ_POLY:
    {
        std::vector<glm::vec3> vertices = boost::get<TrackModel>(raw).m_vertices;
        center                          = boost::get<TrackModel>(raw).initialPosition;
        orientation                     = boost::get<TrackModel>(raw).orientation;
        if (dynamic)
        {
            // btBvhTriangleMeshShape doesn't collide when dynamic, use convex triangle mesh
            auto *mesh = new btTriangleMesh();
            for (size_t vertIdx = 0; vertIdx < vertices.size() - 2; vertIdx += 3)
            {
                glm::vec3 triangle  = vertices[vertIdx];
                glm::vec3 triangle1 = vertices[vertIdx + 1];
                glm::vec3 triangle2 = vertices[vertIdx + 2];
                mesh->addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
            }
            m_collisionShape = new btConvexTriangleMeshShape(mesh);
        }
        else
        {
            // TODO: Use passable flags (flags&0x80) of VROAD to work out whether collidable
            for (size_t vertIdx = 0; vertIdx < vertices.size() - 2; vertIdx += 3)
            {
                glm::vec3 triangle  = vertices[vertIdx];
                glm::vec3 triangle1 = vertices[vertIdx + 1];
                glm::vec3 triangle2 = vertices[vertIdx + 2];
                m_collisionMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
            }
            m_collisionShape = new btBvhTriangleMeshShape(&m_collisionMesh, true, true);
        }
    }
    break;
    }

    float entityMass = dynamic ? 100.f : 0.f;
    btVector3 localInertia;

    if (dynamic)
    {
        m_collisionShape->calculateLocalInertia(entityMass, localInertia);
    }

    m_motionState = new btDefaultMotionState(Utils::MakeTransform(center, orientation));
    rigidBody     = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(entityMass, m_motionState, m_collisionShape, localInertia));
    rigidBody->setFriction(btScalar(1.f));
    rigidBody->setUserPointer(this);
}

void Entity::_GenBoundingBox()
{
    switch (type)
    {
    case XOBJ:
    case OBJ_POLY:
    case LANE:
    case ROAD:
    case GLOBAL:
    {
        DimensionData meshDimensions = Utils::GenDimensions(boost::get<TrackModel>(raw).m_vertices);
        m_boundingBox                = AABB(meshDimensions.minVertex, meshDimensions.maxVertex, boost::get<TrackModel>(raw).initialPosition);
        return;
    }
    case LIGHT:
    {
        // For now, only tracklights will have entities created
        std::shared_ptr<BaseLight> baseLight = boost::get<std::shared_ptr<BaseLight>>(raw);
        ASSERT(baseLight->type == LightType::TRACK_LIGHT, "Not ready to handle other light types at entity creation time");
        std::shared_ptr<TrackLight> trackLight = std::static_pointer_cast<TrackLight>(baseLight);
        DimensionData meshDimensions           = Utils::GenDimensions(trackLight->model.m_vertices);
        m_boundingBox                          = AABB(meshDimensions.minVertex, meshDimensions.maxVertex, baseLight->position);
        return;
    }
    case SOUND:
    case CAR:
    case VROAD:
    case VROAD_CEIL:
        return;
    default:
        ASSERT(false, "Shouldn't be adding a " << ToString(type) << " entity to the AABB tree!");
        break;
    }
}

void Entity::Update()
{
    // We don't want to update Entities that aren't dynamic
    if (!((type == OBJ_POLY || type == XOBJ) && dynamic))
    {
        return;
    }
    btTransform trans;
    m_motionState->getWorldTransform(trans);
    boost::get<TrackModel>(raw).position    = Utils::bulletToGlm(trans.getOrigin());
    boost::get<TrackModel>(raw).orientation = Utils::bulletToGlm(trans.getRotation());
    boost::get<TrackModel>(raw).update();
}

void Entity::_SetCollisionParameters()
{
    switch (tag)
    {
    case NFS_3:
        switch (type)
        {
        case VROAD:
            collideable = true;
            dynamic     = false;
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
            switch ((flags >> 4) & 0x7)
            {
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
        }
        break;
    default:
        collideable = true;
        dynamic     = false;
        // LOG(WARNING) << "Entity parameters are unset for " << ToString(tag);
    }
}

AABB Entity::GetAABB() const
{
    switch (type)
    {
    case SOUND:
    case CAR:
    case VROAD:
    case VROAD_CEIL:
        ASSERT(false, "Shouldn't be adding a " << ToString(type) << " entity to the AABB tree!");
        break;
    default:
        break;
    }

    return m_boundingBox;
}
