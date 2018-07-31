//
// Created by Amrik on 26/02/2018.
//

#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include "TrackBlock.h"

TrackBlock::TrackBlock(int blockID, glm::vec3 center_pos) {
    block_id = blockID;
    center = center_pos;
    engineID = std::to_string(blockID);
}

void TrackBlock::generatePhysicsMesh(){
    // TODO: Use passable flags (flags&0x80), refactor track block into nice data structure. One superset
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

}