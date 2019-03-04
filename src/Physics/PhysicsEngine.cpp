//
// Created by Amrik Sadhra on 05/02/2018.
//


#include "PhysicsEngine.h"

void ScreenPosToWorldRay(
        int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
        int screenWidth, int screenHeight,  // Window size, in pixels
        glm::mat4 ViewMatrix,               // Camera position and orientation
        glm::mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
        glm::vec3 &out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.Reall
        glm::vec3 &out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
) {

    // The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
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

btCollisionShape *PhysicsEngine::buildFrustumShape() {
    float m_ScreenHeight = 768;
    float m_ScreenWidth = 1024;

    const btScalar nearPlane = 1.0f;
    const btScalar farPlane = 1000.0;

    const btScalar planesFraction = farPlane / nearPlane;
    const btScalar centralPlane = (farPlane - nearPlane) * 0.5;    // Actually it's the center of the collision shape.
    btScalar left, right, bottom, top, farLeft, farRight, farBottom, farTop;
    const btScalar aspect = (btScalar) m_ScreenWidth / (btScalar) m_ScreenHeight;
    if (m_ScreenWidth > m_ScreenHeight) {
        left = -aspect;
        right = aspect;
        bottom = -1.0;
        top = 1.0;
    } else {
        left = -1.0;
        right = 1.0;
        bottom = -aspect;
        top = aspect;
    }
    farLeft = left * planesFraction;
    farRight = right * planesFraction;
    farBottom = bottom * planesFraction;
    farTop = top * planesFraction;

    btConvexHullShape *shape = new btConvexHullShape();

    // Add the frustum points
    {
        btVector3 points[8] = {
                btVector3(left, top, centralPlane),
                btVector3(right, top, centralPlane),
                btVector3(left, bottom, centralPlane),
                btVector3(right, bottom, centralPlane),

                btVector3(farLeft, farTop, -centralPlane),
                btVector3(farRight, farTop, -centralPlane),
                btVector3(farLeft, farBottom, -centralPlane),
                btVector3(farRight, farBottom, -centralPlane),
        };

        for (int t = 0; t < 8; t++) {
            shape->addPoint(points[t]);
        }
    }

    // Crashes OpenNFS with a frustum thats actually frustum shaped, use a box for now.
    /*btCompoundShape *frustumShape = new btCompoundShape();
    const btVector3 v(0., 0., -(nearPlane + centralPlane));
    const btQuaternion q = btQuaternion::getIdentity();// = btQuaternion::getIdentity();//(btVector3(0.,1.,0.),btRadians(180));
    btTransform T(q, v);
    frustumShape->addChildShape(T.inverse(), shape);  */
    btBoxShape *frustumShape = new btBoxShape(btVector3(2, 2, 2));

    return frustumShape;
}

void PhysicsEngine::destroyGhostObject() {
    if (!m_ghostObject) return;
    // Remove From World:
    dynamicsWorld->removeCollisionObject(m_ghostObject);
    // Delete Collision Shapes:
    {
        btCollisionShape *mainShape = m_ghostObject->getCollisionShape();
        delete mainShape;
        mainShape = NULL;
    }
    // Delete Ghost Object:
    delete m_ghostObject;
    m_ghostObject = NULL;
}

void PhysicsEngine::buildGhostObject() {
    btCollisionShape *shape = buildFrustumShape();
    destroyGhostObject();

    if (!m_ghostPairCallback) {
        m_ghostPairCallback = new btGhostPairCallback();
        dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(m_ghostPairCallback);
    }

    m_ghostObject = new btPairCachingGhostObject();
    m_ghostObject->setCollisionShape(shape);
    m_ghostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    dynamicsWorld->addCollisionObject(m_ghostObject, btBroadphaseProxy::AllFilter,
                                      COL_TRACK); // btBroadphaseProxy::StaticFilter);
}

void PhysicsEngine::updateFrustrum(glm::mat4 viewMatrix) {
    if (m_ghostObject) m_ghostObject->setWorldTransform(Utils::glmToBullet(viewMatrix).inverse());
    else buildGhostObject();
    checkForFrustumIntersect();
}

void PhysicsEngine::checkForFrustumIntersect() {
    m_objectsInFrustum.resize(0);
    dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(m_ghostObject->getOverlappingPairCache(),
                                                              dynamicsWorld->getDispatchInfo(),
                                                              dynamicsWorld->getDispatcher());

    btBroadphasePairArray &collisionPairs = m_ghostObject->getOverlappingPairCache()->getOverlappingPairArray();    //New
    numObjects = collisionPairs.size();
    static btManifoldArray m_manifoldArray;
    for (int i = 0; i < numObjects; i++) {
        const btBroadphasePair &collisionPair = collisionPairs[i];
        m_manifoldArray.resize(0);
        if (collisionPair.m_algorithm) collisionPair.m_algorithm->getAllContactManifolds(m_manifoldArray);
        else
            std::cerr
                    << "No collisionPair.m_algorithm - probably m_dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(...) must be missing."
                    << std::endl;
        for (int j = 0; j < m_manifoldArray.size(); j++) {
            btPersistentManifold *manifold = m_manifoldArray[j];
            if (manifold->getNumContacts() > 0) {
                m_objectsInFrustum.push_back(
                        (btCollisionObject *) (manifold->getBody0() == m_ghostObject ? manifold->getBody1()
                                                                                     : manifold->getBody0()));
                break;
            }
        }
    }
}

void PhysicsEngine::initSimulation() {
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

void PhysicsEngine::stepSimulation(float time) {
    dynamicsWorld->stepSimulation(time, 100);
    for (auto &car : cars) {
        car->update(dynamicsWorld);
    }
    // TODO: Track updates should only propagate for active track blocks. Active list should be based upon track blocks cars are on
    for (auto &track_block : currentTrack->trackBlocks) {
        for (auto &objects : track_block.objects) {
            //objects.update();
        }
    }
}

void PhysicsEngine::cleanSimulation() {
    for (auto &car : cars) {
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
    delete broadphase;
}

Entity *PhysicsEngine::checkForPicking(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, bool *entityTargeted) {
    glm::vec3 out_origin;
    glm::vec3 out_direction;
    ScreenPosToWorldRay(Config::get().resX / 2, Config::get().resY / 2, Config::get().resX, Config::get().resY,
                        viewMatrix, projectionMatrix, out_origin, out_direction);
    glm::vec3 out_end = out_origin + out_direction * 1000.0f;
    btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(out_origin.x, out_origin.y, out_origin.z),
                                                           btVector3(out_end.x, out_end.y, out_end.z));
    RayCallback.m_collisionFilterMask = COL_CAR | COL_TRACK | COL_DYNAMIC_TRACK;
    dynamicsWorld->rayTest(btVector3(out_origin.x, out_origin.y, out_origin.z),
                           btVector3(out_end.x, out_end.y, out_end.z), RayCallback);
    if (RayCallback.hasHit()) {
        *entityTargeted = true;
        return static_cast<Entity *>(RayCallback.m_collisionObject->getUserPointer());
    } else {
        *entityTargeted = false;
        return nullptr;
    }
}

void PhysicsEngine::registerTrack(const std::shared_ptr<ONFSTrack> &track) {
    currentTrack = track;
    for (auto &trackBlock : track->trackBlocks) {
        for (auto &road : trackBlock.track) {
            road.genPhysicsMesh();
            dynamicsWorld->addRigidBody(road.rigidBody, COL_TRACK, COL_CAR | COL_RAY | COL_DYNAMIC_TRACK);
        }
        for (auto &object : trackBlock.objects) {
            object.genPhysicsMesh();
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
            light.genPhysicsMesh();
            dynamicsWorld->addRigidBody(light.rigidBody, COL_TRACK, COL_RAY);
        }
    }
    if (track->tag == NFS_3 || track->tag == NFS_4) {
        uint32_t nVroad = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroadHead.nrec;
        for (uint32_t vroad_Idx = 0; vroad_Idx < nVroad; ++vroad_Idx) {
            if (vroad_Idx < nVroad - 1) {
                COLVROAD curVroad = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[vroad_Idx];
                COLVROAD nextVroad = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[vroad_Idx + 1];
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
                Entity leftVroadBarrier = Entity(99, 99, NFSVer::NFS_3, VROAD, curLeftVroadEdge, nextLeftVroadEdge);
                Entity rightVroadBarrier = Entity(99, 99, NFSVer::NFS_3, VROAD, curRightVroadEdge, nextRightVroadEdge);
                leftVroadBarrier.genPhysicsMesh();
                rightVroadBarrier.genPhysicsMesh();
                dynamicsWorld->addRigidBody(leftVroadBarrier.rigidBody, COL_TRACK, COL_RAY | COL_CAR);
                dynamicsWorld->addRigidBody(rightVroadBarrier.rigidBody, COL_TRACK, COL_RAY | COL_CAR);

                // Keep track of them so can clean up later
                track->vroadBarriers.emplace_back(leftVroadBarrier);
                track->vroadBarriers.emplace_back(rightVroadBarrier);
            }
        }
    }
}

void PhysicsEngine::registerVehicle(std::shared_ptr<Car> &car) {
    cars.emplace_back(car);

    btVector3 wheelDirectionCS0(0, -1, 0);
    btVector3 wheelAxleCS(-1, 0, 0);
    float wheelRadius = car->getWheelRadius();
    float wheelWidth = car->getWheelWidth();
    btScalar sRestLength = car->getSuspensionRestLength();

    dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(car->getVehicleRigidBody()->getBroadphaseHandle(), dynamicsWorld->getDispatcher());
    dynamicsWorld->addRigidBody(car->getVehicleRigidBody(), COL_CAR, COL_TRACK | COL_RAY | COL_DYNAMIC_TRACK);
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
    initSimulation();
}

