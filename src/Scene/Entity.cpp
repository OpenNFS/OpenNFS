//
// Created by Amrik Sadhra on 31/07/2018.
//


#include "Entity.h"

Entity::Entity(uint32_t parent_trackblock_id, uint32_t entity_id, NFSVer nfs_version, EntityType entity_type, EngineModel gl_mesh, uint32_t flags) {
    tag = nfs_version;
    type = entity_type;
    glMesh = gl_mesh;
    this->flags = flags;
    parentTrackblockID = parent_trackblock_id;
    entityID = entity_id;
    setParameters();
}

void Entity::genPhysicsMesh(){
    if(type == LIGHT){
        // Light mesh billboarded, generated AABB too large. Divide verts by scale factor to make smaller.
        std::vector<glm::vec3> vertices = boost::get<Light>(glMesh).m_vertices;
        glm::vec3 lightPosition =  boost::get<Light>(glMesh).position;
        float lightBoundScaleF = 10.f;
        for(int i = 0; i < vertices.size()-2; i+=3){
            glm::vec3 triangle = glm::vec3((   vertices[i].x/lightBoundScaleF) + lightPosition.x, (  vertices[i].y/lightBoundScaleF) + lightPosition.y,(  vertices[i].z/lightBoundScaleF) + lightPosition.z);
            glm::vec3 triangle1= glm::vec3((vertices[i+1].x /lightBoundScaleF) + lightPosition.x, (vertices[i+1].y/lightBoundScaleF) + lightPosition.y,(vertices[i+1].z/lightBoundScaleF) + lightPosition.z);
            glm::vec3 triangle2= glm::vec3((vertices[i+2].x /lightBoundScaleF) + lightPosition.x, (vertices[i+2].y/lightBoundScaleF) + lightPosition.y,(vertices[i+2].z/lightBoundScaleF) + lightPosition.z);
            physicsMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
        }
    } else {
        std::vector<glm::vec3>vertices = boost::get<Track>(glMesh).m_vertices;
        // TODO: Use passable flags (flags&0x80) of VROAD to work out whether collidable
        for(int i = 0; i < vertices.size()-2; i+=3){
            glm::vec3 triangle = glm::vec3(  vertices[i].x,   vertices[i].y,   vertices[i].z);
            glm::vec3 triangle1= glm::vec3(vertices[i+1].x, vertices[i+1].y, vertices[i+1].z);
            glm::vec3 triangle2= glm::vec3(vertices[i+2].x, vertices[i+2].y, vertices[i+2].z);
            physicsMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
        }
    }
    physicsShape = new btBvhTriangleMeshShape(&physicsMesh, true, true);
    motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0,0,0)));
    rigidBody = new btRigidBody( btRigidBody::btRigidBodyConstructionInfo(0, motionState, physicsShape, btVector3(0, 0, 0)));
    rigidBody->setFriction(btScalar(1.f));
    rigidBody->setUserPointer(this);
}

void Entity::setParameters() {
    switch(tag){
        case NFS_3:
            switch(type){
                case LIGHT:
                    collideable = false;
                    break;

                case SOUND:
                    collideable = false;
                    break;
                case OBJ_POLY:
                    if(flags & (1 << 4)){ // Flags 32 for hometown is a godray
                        collideable = false;
                    } else {
                        collideable = true;
                    }
                    break;
            }
            break;
        default:
            collideable = true;
            LOG(WARNING) << "Entity parameters are unset for " << ToString(tag);
    }
}