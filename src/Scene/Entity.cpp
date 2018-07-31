//
// Created by SYSTEM on 31/07/2018.
//


#include "Entity.h"

Entity::Entity(uint32_t parent_trackblock_id, uint32_t entity_id, NFSVer nfs_version, EntityType entity_type, EngineModel gl_mesh) {
    tag = nfs_version;
    type = entity_type;
    glMesh = gl_mesh;
}

void Entity::genPhysicsMesh(){
    Model *model;
    if(type == LIGHT){
        model = static_cast<Model *>(boost::get<shared_ptr<Light>>(glMesh).get());
    } else {
        model = static_cast<Model *>(boost::get<shared_ptr<Track>>(glMesh).get());
    }
    // TODO: Use passable flags (flags&0x80), refactor track block into nice data structure. One superset
    for(int i = 0; i < model->m_vertices.size()-2; i+=3){
        glm::vec3 triangle = glm::vec3(model->m_vertices[i].x, model->m_vertices[i].y, model->m_vertices[i].z);
        glm::vec3 triangle1= glm::vec3(model->m_vertices[i+1].x, model->m_vertices[i+1].y, model->m_vertices[i+1].z);
        glm::vec3 triangle2= glm::vec3(model->m_vertices[i+2].x, model->m_vertices[i+2].y, model->m_vertices[i+2].z);
        physicsMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
    }
    physicsShape = new btBvhTriangleMeshShape(&physicsMesh, true, true);
    motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0,0,0)));
    rigidBody = new btRigidBody( btRigidBody::btRigidBodyConstructionInfo(0, motionState, physicsShape, btVector3(0, 0, 0)));
    rigidBody->setFriction(btScalar(1.f));
    rigidBody->setUserPointer(this);
}