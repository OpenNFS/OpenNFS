//
// Created by SYSTEM on 31/07/2018.
//

#include "Entity.h"

Entity::Entity(uint32_t parent_trackblock_id, uint32_t entity_id, NFSVer nfs_version, EntityType entity_type, EngineModel gl_mesh) {
    tag = nfs_version;
    type = entity_type;
    glMesh = gl_mesh;

    /*btTriangleMesh physicsMesh;
    btRigidBody* rigidBody;
    btCollisionShape* physicsShape;
    btDefaultMotionState* motionState;
*/}

void Entity::genPhysicsMesh(){
    /*// TODO: Use passable flags (flags&0x80), refactor track block into nice data structure. One superset
    for (auto &track_block_model : track) {
        for(int i = 0; i < track_block_model.m_vertices.size()-2; i+=3){
            glm::vec3 triangle = glm::vec3(track_block_model.m_vertices[i].x, track_block_model.m_vertices[i].y, track_block_model.m_vertices[i].z);
            glm::vec3 triangle1= glm::vec3(track_block_model.m_vertices[i+1].x, track_block_model.m_vertices[i+1].y, track_block_model.m_vertices[i+1].z);
            glm::vec3 triangle2= glm::vec3(track_block_model.m_vertices[i+2].x, track_block_model.m_vertices[i+2].y, track_block_model.m_vertices[i+2].z);
            trackMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
        }
    }
    trackShape = new btBvhTriangleMeshShape(&trackMesh, true, true);
    groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0,0,0)));
    trackRigidBody = new btRigidBody( btRigidBody::btRigidBodyConstructionInfo(0, groundMotionState, trackShape, btVector3(0, 0, 0)));
    trackRigidBody->setFriction(btScalar(1.f));
    trackRigidBody->setUserPointer(this);
    // Need an array of rigid bodies for each XOBJ I think
*/
}