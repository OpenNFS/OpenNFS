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
    dynamicsWorld->stepSimulation(time, 15);
    for(auto &car : cars){
        car->update();
    }
}

void Physics::cleanSimulation() {
    // TODO: Only cleanup if have loaded these guys
    for(auto &car : cars){
        //dynamicsWorld->removeRigidBody(car->m);
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
    btCollisionShape* trackShape =new btBvhTriangleMeshShape(&trackMesh, true, true); // new btStaticPlaneShape(btVector3(0,1,0),1);//
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0,0,0)));
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, trackShape, btVector3(0, 0, 0));
    groundRigidBody = new btRigidBody(groundRigidBodyCI);
    groundRigidBody->setFriction(btScalar(2.0f));
    dynamicsWorld->addRigidBody(groundRigidBody);
}

void Physics::registerVehicle(Car *car) {
    cars.emplace_back(car);

    btVector3 wheelDirectionCS0(0,-1,0);
    btVector3 wheelAxleCS(-1,0,0);
    float	wheelRadius = car -> getWheelRadius();
    float	wheelWidth = car -> getWheelWidth();
    btScalar sRestLength = car -> getSuspensionRestLength();

    dynamicsWorld -> getBroadphase()-> getOverlappingPairCache() ->
            cleanProxyFromPairs(
            car -> getVehicleRigidBody() -> getBroadphaseHandle(),
            dynamicsWorld -> getDispatcher()
    );

    dynamicsWorld -> addRigidBody (car -> getVehicleRigidBody());
    car->m_vehicleRayCaster = new btDefaultVehicleRaycaster(dynamicsWorld);
    car->m_vehicle = new btRaycastVehicle(car->m_tuning, car->getVehicleRigidBody(), car->getRaycaster());
    car -> getVehicleRigidBody() -> setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld -> addVehicle(car -> m_vehicle);
    car->getRaycast() -> setCoordinateSystem(0,1,2);

    // Wire up the wheels
    // Fronties
    btVector3 connectionPointCS0(Utils::glmToBullet(car->car_models[0].position - car->car_models[3].position));
    car->getRaycast() -> addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,sRestLength,wheelRadius,car -> m_tuning, true);
    connectionPointCS0 = btVector3(Utils::glmToBullet(car->car_models[0].position - car->car_models[4].position));
    car->getRaycast() -> addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,sRestLength,wheelRadius,car -> m_tuning, true);

    // Rearies
    connectionPointCS0 = btVector3(Utils::glmToBullet(car->car_models[0].position - car->car_models[1].position));
    car->getRaycast() -> addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,sRestLength,wheelRadius,car -> m_tuning,false);
    connectionPointCS0 = btVector3(Utils::glmToBullet(car->car_models[0].position - car->car_models[2].position));
    car->getRaycast() -> addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,sRestLength,wheelRadius,car -> m_tuning,false);

    for (int i = 0; i < car->getRaycast()->getNumWheels(); i++)
    {
        btWheelInfo& wheel = car -> getRaycast() -> getWheelInfo(i);
        wheel.m_suspensionStiffness = car -> getSuspensionStiffness();
        wheel.m_wheelsDampingRelaxation = car -> getSuspensionDamping();
        wheel.m_wheelsDampingCompression = car -> getSuspensionCompression();
        wheel.m_frictionSlip = car -> getWheelFriction();
        wheel.m_rollInfluence = car -> getRollInfluence();
    }
}

Physics::Physics() {
    initSimulation();
}

