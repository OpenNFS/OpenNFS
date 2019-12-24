#include "PhysicsEngine.h"

WorldRay ScreenPosToWorldRay(int mouseX, int mouseY, int screenWidth, int screenHeight, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix) {
    // The ray Start and End positions, in Normalized Device Coordinates
    glm::vec4 lRayStart_NDC(
            ((float) mouseX / (float) screenWidth - 0.5f) * 2.0f,
            ((float) mouseY / (float) screenHeight - 0.5f) * 2.0f,
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

    // Faster way (just one inverse)
    glm::mat4 M = glm::inverse(ProjectionMatrix * ViewMatrix);
    glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
    glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;

    glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
    lRayDir_world = glm::normalize(lRayDir_world);

    WorldRay worldRay;
    worldRay.origin = glm::vec3(lRayStart_world);
    worldRay.direction - glm::normalize(lRayDir_world);

    return worldRay;
}

void PhysicsEngine::InitSimulation() {
    /*------- BULLET --------*/
    m_broadphase = new btDbvtBroadphase();
    // Set up the collision configuration and dispatcher
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    // The actual physics solver
    solver = new btSequentialImpulseConstraintSolver;
    // The world.
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, m_broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
    dynamicsWorld->setDebugDrawer(&debugDrawer);
}

void PhysicsEngine::StepSimulation(float time) {
    dynamicsWorld->stepSimulation(time, 100);
    for (auto &car : m_activeVehicles) {
        car->update(dynamicsWorld);
    }
    if(currentTrack.get() != nullptr)
    {
        // TODO: Track updates should only propagate for active track blocks. Active list should be based upon track blocks cars are on
        for (auto &track_block : currentTrack->trackBlocks) {
            for (auto &objects : track_block.objects) {
                //objects.update();
            }
        }
    }
}

Entity *PhysicsEngine::CheckForPicking(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, bool *entityTargeted) {
    WorldRay worldRayFromScreenPosition = ScreenPosToWorldRay(Config::get().resX / 2, Config::get().resY / 2, Config::get().resX, Config::get().resY, viewMatrix, projectionMatrix);
    glm::vec3 outEnd = worldRayFromScreenPosition.origin + worldRayFromScreenPosition.direction * 1000.0f;
    btCollisionWorld::ClosestRayResultCallback rayCallback(Utils::glmToBullet(worldRayFromScreenPosition.origin), Utils::glmToBullet(outEnd));
    rayCallback.m_collisionFilterMask = COL_CAR | COL_TRACK | COL_DYNAMIC_TRACK;
    dynamicsWorld->rayTest(Utils::glmToBullet(worldRayFromScreenPosition.origin), Utils::glmToBullet(outEnd), rayCallback);
    if (rayCallback.hasHit()) {
        *entityTargeted = true;
        return static_cast<Entity *>(rayCallback.m_collisionObject->getUserPointer());
    } else {
        *entityTargeted = false;
        return nullptr;
    }
}

void PhysicsEngine::RegisterTrack(const std::shared_ptr<ONFSTrack> &track) {
    currentTrack = track;
    for (auto &trackBlock : track->trackBlocks) {
        for (auto &road : trackBlock.track) {
            dynamicsWorld->addRigidBody(road.rigidBody, COL_TRACK, COL_CAR | COL_RAY | COL_DYNAMIC_TRACK);
        }
        for (auto &object : trackBlock.objects) {
            int collisionMask = COL_RAY;
            // Set collision masks
            if (object.collideable) {
                collisionMask |= COL_CAR;
            }
            if (object.dynamic){
                collisionMask |= COL_TRACK;
            }
            // Move Rigid body to correct place in world
            btTransform initialTransform;
            initialTransform.setOrigin(Utils::glmToBullet(boost::get<Track>(object.glMesh).initialPosition));
            initialTransform.setRotation(Utils::glmToBullet(boost::get<Track>(object.glMesh).orientation));
            object.rigidBody->setWorldTransform(initialTransform);
            dynamicsWorld->addRigidBody(object.rigidBody, COL_DYNAMIC_TRACK, collisionMask);
        }
        for (auto &light : trackBlock.lights) {
            dynamicsWorld->addRigidBody(light.rigidBody, COL_TRACK, COL_RAY);
        }
    }
    if ((track->tag == NFS_3 || track->tag == NFS_4) && !Config::get().sparkMode) {
        uint32_t nVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroadHead.nrec;
        for (uint32_t vroad_Idx = 0; vroad_Idx < nVroad; ++vroad_Idx) {
            if (vroad_Idx < nVroad - 1) {
                COLVROAD curVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[vroad_Idx];
                COLVROAD nextVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[vroad_Idx + 1];
                INTPT curVroadRefPt = curVroad.refPt;
                INTPT nextVroadRefPt = nextVroad.refPt;

                glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));

                // Transform NFS3/4 coords into ONFS 3d space
                glm::vec3 curVroadPoint = rotationMatrix * glm::vec3((curVroadRefPt.x / 65536.0f) / 10.f, ((curVroadRefPt.y / 65536.0f) / 10.f), (curVroadRefPt.z / 65536.0f) / 10.f);
                glm::vec3 nextVroadPoint = rotationMatrix * glm::vec3((nextVroadRefPt.x / 65536.0f) / 10.f, ((nextVroadRefPt.y / 65536.0f) / 10.f), (nextVroadRefPt.z / 65536.0f) / 10.f);

                // Get VROAD right vector
                glm::vec3 curVroadRightVec = rotationMatrix * glm::vec3(curVroad.right.x / 128.f, curVroad.right.y / 128.f, curVroad.right.z / 128.f);
                glm::vec3 nextVroadRightVec = rotationMatrix * glm::vec3(nextVroad.right.x / 128.f, nextVroad.right.y / 128.f, nextVroad.right.z / 128.f);


                glm::vec3 curVroadLeftWall = ((curVroad.leftWall/65536.0f) / 10.f) * curVroadRightVec;
                glm::vec3 curVroadRightWall = ((curVroad.rightWall/65536.0f) / 10.f) * curVroadRightVec;
                glm::vec3 nextVroadLeftWall = ((nextVroad.leftWall/65536.0f) / 10.f) * nextVroadRightVec;
                glm::vec3 nextVroadRightWall = ((nextVroad.rightWall/65536.0f) / 10.f) * nextVroadRightVec;

                bool useFullVroad = Config::get().useFullVroad;

                // Get edges of road by adding to vroad right vector to vroad reference point
                glm::vec3 curLeftVroadEdge = curVroadPoint      - (useFullVroad ? curVroadLeftWall : curVroadRightVec);
                glm::vec3 curRightVroadEdge = curVroadPoint     + (useFullVroad ? curVroadRightWall : curVroadRightVec);
                glm::vec3 nextLeftVroadEdge = nextVroadPoint    - (useFullVroad ? nextVroadLeftWall : nextVroadRightVec);
                glm::vec3 nextRightVroadEdge = nextVroadPoint   + (useFullVroad ? nextVroadRightWall : nextVroadRightVec);

                // Add them to the physics world
                Entity leftVroadBarrier = Entity(99, 99, NFSVer::NFS_3, VROAD, nullptr, 0, curLeftVroadEdge, nextLeftVroadEdge);
                Entity rightVroadBarrier = Entity(99, 99, NFSVer::NFS_3, VROAD, nullptr, 0, curRightVroadEdge, nextRightVroadEdge);
                Entity vroadCeiling = Entity(99, 99, NFSVer::NFS_3, VROAD_CEIL, nullptr, 0, curLeftVroadEdge, nextLeftVroadEdge, curRightVroadEdge, nextRightVroadEdge);
                dynamicsWorld->addRigidBody(leftVroadBarrier.rigidBody,  COL_VROAD,  COL_RAY | COL_CAR);
                dynamicsWorld->addRigidBody(rightVroadBarrier.rigidBody, COL_VROAD, COL_RAY | COL_CAR);
                dynamicsWorld->addRigidBody(vroadCeiling.rigidBody, COL_VROAD_CEIL, COL_RAY);

                // Keep track of them so can clean up later
                track->vroadBarriers.emplace_back(leftVroadBarrier);
                track->vroadBarriers.emplace_back(rightVroadBarrier);
                track->vroadBarriers.emplace_back(vroadCeiling);
            }
        }
    }
}

void PhysicsEngine::RegisterVehicle(std::shared_ptr<Car> &car) {
    m_activeVehicles.emplace_back(car);

    btVector3 wheelDirectionCS0(0, -1, 0);
    btVector3 wheelAxleCS(-1, 0, 0);
    float wheelRadius = car->getWheelRadius();
    float wheelWidth = car->getWheelWidth();
    btScalar sRestLength = car->getSuspensionRestLength();

    dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(car->getVehicleRigidBody()->getBroadphaseHandle(), dynamicsWorld->getDispatcher());
    dynamicsWorld->addRigidBody(car->getVehicleRigidBody(), COL_CAR, COL_TRACK | COL_RAY | COL_DYNAMIC_TRACK | COL_VROAD);
    car->m_vehicleRayCaster = new btDefaultVehicleRaycaster(dynamicsWorld);
    car->m_vehicle = new btRaycastVehicle(car->m_tuning, car->getVehicleRigidBody(), car->getRaycaster());
    car->getVehicleRigidBody()->setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld->addVehicle(car->m_vehicle);
    car->getRaycast()->setCoordinateSystem(0, 1, 2);

    // Wire up the wheels
    // Fronties
    btVector3 connectionPointCS0(Utils::glmToBullet(car->leftFrontWheelModel.position));
    car->getRaycast()->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, sRestLength, wheelRadius,
                                car->m_tuning, true);
    connectionPointCS0 = btVector3(Utils::glmToBullet(car->rightFrontWheelModel.position));
    car->getRaycast()->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, sRestLength, wheelRadius,
                                car->m_tuning, true);
    // Rearies
    connectionPointCS0 = btVector3(Utils::glmToBullet(car->leftRearWheelModel.position));
    car->getRaycast()->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, sRestLength, wheelRadius,
                                car->m_tuning, false);
    connectionPointCS0 = btVector3(Utils::glmToBullet(car->rightRearWheelModel.position));
    car->getRaycast()->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, sRestLength, wheelRadius,
                                car->m_tuning, false);

    for (int i = 0; i < car->getRaycast()->getNumWheels(); i++) {
        btWheelInfo &wheel = car->getRaycast()->getWheelInfo(i);
        wheel.m_suspensionStiffness = car->getSuspensionStiffness();
        wheel.m_wheelsDampingRelaxation = car->getSuspensionDamping();
        wheel.m_wheelsDampingCompression = car->getSuspensionCompression();
        wheel.m_frictionSlip = car->getWheelFriction();
        wheel.m_rollInfluence = car->getRollInfluence();
    }
}

PhysicsEngine::PhysicsEngine() {
    InitSimulation();
}

PhysicsEngine::~PhysicsEngine() {
    for (auto &car : m_activeVehicles) {
        dynamicsWorld->removeRigidBody(car->getVehicleRigidBody());
    }
    for (auto &trackBlock : currentTrack->trackBlocks) {
        for (auto &road : trackBlock.track) {
            dynamicsWorld->removeRigidBody(road.rigidBody);
            delete road.rigidBody->getMotionState();
            delete road.rigidBody;
        }
        for (auto &object : trackBlock.objects) {
            dynamicsWorld->removeRigidBody(object.rigidBody);
            delete object.rigidBody->getMotionState();
            delete object.rigidBody;
        }
        for (auto &light : trackBlock.lights) {
            dynamicsWorld->removeRigidBody(light.rigidBody);
            delete light.rigidBody->getMotionState();
            delete light.rigidBody;
        }
    }
    for(auto &vroadBarrier : currentTrack->vroadBarriers){
        dynamicsWorld->removeRigidBody(vroadBarrier.rigidBody);
        delete vroadBarrier.rigidBody->getMotionState();
        delete vroadBarrier.rigidBody;
    }
    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete m_broadphase;
}

