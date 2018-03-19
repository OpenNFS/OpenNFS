//
// Created by Amrik Sadhra on 05/02/2018.
//


#include "Physics.h"

void Physics::initSimulation() {
    /*------- BULLET --------*/
    broadphase = new btDbvtBroadphase();
    // Set up the collision configuration and dispatcher
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    // The actual physics solver
    solver = new btSequentialImpulseConstraintSolver;
    // The world.
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
    dynamicsWorld->setDebugDrawer(&mydebugdrawer);
}

void Physics::stepSimulation(float time) {
    dynamicsWorld->stepSimulation(time/10, 10);
    for(auto &car : cars){
        car->update();
    }
}

void Physics::cleanSimulation() {
    // TODO: Only cleanup if have loaded these guys
    for(auto &car : cars){
        dynamicsWorld->removeRigidBody(car->bodyRigidBody);
        dynamicsWorld->removeRigidBody(car->fl_TireRigidBody);
        dynamicsWorld->removeRigidBody(car->fr_TireRigidBody);
        dynamicsWorld->removeRigidBody(car->bl_TireRigidBody);
        dynamicsWorld->removeRigidBody(car->br_TireRigidBody);
    }
    dynamicsWorld->removeRigidBody(groundRigidBody);
    delete groundRigidBody->getMotionState();
    delete groundRigidBody;
    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
}

void Physics::registerTrack(const std::vector<TrackBlock> &track_blocks){
    // TODO: Use passable flags (flags&0x80), refactor track block into nice data structure. One superset
    for (auto &track_block : track_blocks) {
        for (auto &track_block_model : track_block.track) {
            glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-SIMD_PI/2,0,0)));
            for(int i = 0; i < track_block_model.m_vertices.size()-2; i+=3){
                glm::vec3 triangle = orientation * glm::vec3(track_block_model.m_vertices[i].x, track_block_model.m_vertices[i].y, track_block_model.m_vertices[i].z);
                glm::vec3 triangle1= orientation * glm::vec3(track_block_model.m_vertices[i+1].x, track_block_model.m_vertices[i+1].y, track_block_model.m_vertices[i+1].z);
                glm::vec3 triangle2= orientation * glm::vec3(track_block_model.m_vertices[i+2].x, track_block_model.m_vertices[i+2].y, track_block_model.m_vertices[i+2].z);
                trackMesh.addTriangle(Utils::glmToBullet(triangle), Utils::glmToBullet(triangle1), Utils::glmToBullet(triangle2), false);
            }
        }
    }
    btCollisionShape* trackShape = new btBvhTriangleMeshShape(&trackMesh, true, true);
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0,0,0)));
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, trackShape, btVector3(0, 0, 0));
    groundRigidBody = new btRigidBody(groundRigidBodyCI);
    groundRigidBody->setFriction(btScalar(0.9));
    dynamicsWorld->addRigidBody(groundRigidBody);
}

void Physics::registerVehicle(Car *car) {
    cars.emplace_back(car);
    dynamicsWorld->addRigidBody(car->bodyRigidBody);
    dynamicsWorld->addRigidBody(car->fl_TireRigidBody);
    dynamicsWorld->addRigidBody(car->fr_TireRigidBody);
    dynamicsWorld->addRigidBody(car->bl_TireRigidBody);
    dynamicsWorld->addRigidBody(car->br_TireRigidBody);
}

Physics::Physics() {
    initSimulation();
}

