//
// Created by Amrik Sadhra on 05/02/2018.
//


#include "Physics.h"

void ScreenPosToWorldRay(
        int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
        int screenWidth, int screenHeight,  // Window size, in pixels
        glm::mat4 ViewMatrix,               // Camera position and orientation
        glm::mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
        glm::vec3 &out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
        glm::vec3 &out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
) {

    // The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
    glm::vec4 lRayStart_NDC(
            ((float) mouseX / (float) screenWidth - 0.5f) * 2.0f, // [0,1024] -> [-1,1]
            ((float) mouseY / (float) screenHeight - 0.5f) * 2.0f, // [0, 768] -> [-1,1]
            -1.0, // The near plane maps to Z=-1 in Normalized Device Coordinates
            1.0f
    );
    glm::vec4 lRayEnd_NDC(
            ((float) mouseX / (float) screenWidth - 0.5f) * 2.0f,
            ((float) mouseY / (float) screenHeight - 0.5f) * 2.0f,
            0.0,
            1.0f
    );


    // The Projection matrix goes from Camera Space to NDC.
    // So inverse(ProjectionMatrix) goes from NDC to Camera Space.
    glm::mat4 InverseProjectionMatrix = glm::inverse(ProjectionMatrix);

    // The View Matrix goes from World Space to Camera Space.
    // So inverse(ViewMatrix) goes from Camera Space to World Space.
    glm::mat4 InverseViewMatrix = glm::inverse(ViewMatrix);

    glm::vec4 lRayStart_camera = InverseProjectionMatrix * lRayStart_NDC;
    lRayStart_camera /= lRayStart_camera.w;
    glm::vec4 lRayStart_world = InverseViewMatrix * lRayStart_camera;
    lRayStart_world /= lRayStart_world.w;
    glm::vec4 lRayEnd_camera = InverseProjectionMatrix * lRayEnd_NDC;
    lRayEnd_camera /= lRayEnd_camera.w;
    glm::vec4 lRayEnd_world = InverseViewMatrix * lRayEnd_camera;
    lRayEnd_world /= lRayEnd_world.w;


    // Faster way (just one inverse)
    //glm::mat4 M = glm::inverse(ProjectionMatrix * ViewMatrix);
    //glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
    //glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;

    glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
    lRayDir_world = glm::normalize(lRayDir_world);


    out_origin = glm::vec3(lRayStart_world);
    out_direction = glm::normalize(lRayDir_world);
}

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
        //dynamicsWorld->removeRigidBody(car->);
    }
    for (auto &track_block : current_track->track_blocks) {
        for(auto &road : track_block.track){
            dynamicsWorld->removeRigidBody(road.rigidBody);
            delete road.rigidBody->getMotionState();
            delete road.rigidBody;
        }
        for(auto &object : track_block.objects){
            dynamicsWorld->removeRigidBody(object.rigidBody);
            delete object.rigidBody->getMotionState();
            delete object.rigidBody;
        }
        for(auto &light : track_block.lights){
            dynamicsWorld->removeRigidBody(light.rigidBody);
            delete light.rigidBody->getMotionState();
            delete light.rigidBody;
        }
    }
    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
}

void Physics::registerTrack(const std::shared_ptr<ONFSTrack> &track){
    current_track = track;
    // TODO: Use passable flags (flags&0x80), refactor track block into nice data structure. One superset
    for (auto &track_block : track->track_blocks) {
        for(auto &road : track_block.track){
            road.genPhysicsMesh();
            dynamicsWorld->addRigidBody(road.rigidBody);
        }
        for(auto &object : track_block.objects){
            object.genPhysicsMesh();
            dynamicsWorld->addRigidBody(object.rigidBody);
        }
        for(auto &light : track_block.lights){
            light.genPhysicsMesh();
            dynamicsWorld->addRigidBody(light.rigidBody);
        }
    }
}

void Physics::registerVehicle(std::shared_ptr<Car> car) {
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

