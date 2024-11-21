#include "PhysicsEngine.h"

#include "CollisionMasks.h"

namespace OpenNFS {
    WorldRay ScreenPosToWorldRay(int mouseX, int mouseY, int screenWidth, int screenHeight, glm::mat4 ViewMatrix,
                                 glm::mat4 ProjectionMatrix) {
        // The ray Start and End positions, in Normalized Device Coordinates
        glm::vec4 lRayStart_NDC(((float) mouseX / (float) screenWidth - 0.5f) * 2.0f,
                                ((float) mouseY / (float) screenHeight - 0.5f) * 2.0f,
                                -1.0, // The near plane maps to Z=-1 in Normalized Device Coordinates
                                1.0f);
        glm::vec4 lRayEnd_NDC(((float) mouseX / (float) screenWidth - 0.5f) * 2.0f,
                              ((float) mouseY / (float) screenHeight - 0.5f) * 2.0f, 0.0, 1.0f);

        // The Projection matrix goes from Camera Space to NDC.
        // So inverse(ProjectionMatrix) goes from NDC to Camera Space.
        glm::mat4 InverseProjectionMatrix = glm::inverse(ProjectionMatrix);

        // The View Matrix goes from World Space to Camera Space.
        // So inverse(ViewMatrix) goes from Camera Space to World Space.
        glm::mat4 InverseViewMatrix = glm::inverse(ViewMatrix);

        // Faster way (just one inverse)
        glm::mat4 M = glm::inverse(ProjectionMatrix * ViewMatrix);
        glm::vec4 lRayStart_world = M * lRayStart_NDC;
        lRayStart_world /= lRayStart_world.w;
        glm::vec4 lRayEnd_world = M * lRayEnd_NDC;
        lRayEnd_world /= lRayEnd_world.w;

        glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
        lRayDir_world = glm::normalize(lRayDir_world);

        WorldRay worldRay{glm::vec3(lRayStart_world), glm::normalize(lRayDir_world)};

        return worldRay;
    }

    PhysicsEngine::PhysicsEngine() : debugDrawer(std::make_shared<BulletDebugDrawer>()) {
        m_pBroadphase = std::make_unique<btDbvtBroadphase>();
        // Set up the collision configuration and dispatcher
        m_pCollisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
        m_pDispatcher = std::make_unique<btCollisionDispatcher>(m_pCollisionConfiguration.get());
        // The actual physics solver
        m_pSolver = std::make_unique<btSequentialImpulseConstraintSolver>();
        // The world.
        m_pDynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(m_pDispatcher.get(), m_pBroadphase.get(),
                                                                     m_pSolver.get(), m_pCollisionConfiguration.get());
        m_pDynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
        m_pDynamicsWorld->setDebugDrawer(debugDrawer.get());
    }

    void PhysicsEngine::StepSimulation(float time, const std::vector<uint32_t> &racerResidentTrackblockIDs) {
        m_pDynamicsWorld->stepSimulation(time, 100);

        for (const auto &car: m_activeVehicles) {
            car->Update(m_pDynamicsWorld.get());
        }

        // TODO: TrackModel updates should only propagate for active track blocks, based upon track blocks racer vehicles are on
        for (const auto &entity: m_track->entities) {
            entity->Update();
        }
    }

    std::optional<Entity *> PhysicsEngine::CheckForPicking(const glm::mat4 &viewMatrix,
                                                           const glm::mat4 &projectionMatrix) {
        WorldRay worldRayFromScreenPosition =
                ScreenPosToWorldRay(Config::get().resX / 2, Config::get().resY / 2, Config::get().resX,
                                    Config::get().resY, viewMatrix, projectionMatrix);
        const glm::vec3 outEnd{worldRayFromScreenPosition.origin + worldRayFromScreenPosition.direction * 1000.0f};

        btCollisionWorld::ClosestRayResultCallback rayCallback(Utils::glmToBullet(worldRayFromScreenPosition.origin),
                                                               Utils::glmToBullet(outEnd));
        rayCallback.m_collisionFilterMask = COL_CAR | COL_TRACK | COL_DYNAMIC_TRACK;

        m_pDynamicsWorld->rayTest(Utils::glmToBullet(worldRayFromScreenPosition.origin), Utils::glmToBullet(outEnd),
                                  rayCallback);

        Entity *targetedEntity = rayCallback.hasHit()
                                     ? (Entity *) rayCallback.m_collisionObject->getUserPointer()
                                     : nullptr;
        return targetedEntity == nullptr
                   ? std::optional<Entity *>(std::nullopt)
                   : std::optional<Entity *>(targetedEntity);
    }

    void PhysicsEngine::RegisterTrack(const std::shared_ptr<Track> &track) {
        m_track = track;

        for (const auto &entity: m_track->entities) {
            int collisionMask = COL_RAY | COL_CAR;
            if (!entity->track_entity->collideable) {
                continue;
            }
            if (entity->track_entity->dynamic) {
                collisionMask |= COL_TRACK;
                // Move Rigid body to correct place in world
                btTransform initialTransform = Utils::MakeTransform(entity->model->geometry->initialPosition,
                                                                    entity->model->geometry->orientation);
                entity->rigidBody->setWorldTransform(initialTransform);
                m_pDynamicsWorld->addRigidBody(entity->rigidBody.get(), COL_DYNAMIC_TRACK, collisionMask);
            } else {
                m_pDynamicsWorld->addRigidBody(entity->rigidBody.get(), COL_TRACK, collisionMask);
            }
        }
    }

    void PhysicsEngine::RegisterVehicle(const std::shared_ptr<Car> &car) {
        car->SetRaycaster(std::make_unique<btDefaultVehicleRaycaster>(m_pDynamicsWorld.get()));
        car->SetVehicle(
            std::make_unique<btRaycastVehicle>(car->tuning, car->GetVehicleRigidBody(), car->GetRaycaster()));
        car->GetVehicle()->setCoordinateSystem(0, 1, 2);

        m_pDynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(
            car->GetVehicleRigidBody()->getBroadphaseHandle(), m_pDynamicsWorld->getDispatcher());
        m_pDynamicsWorld->addRigidBody(car->GetVehicleRigidBody(), COL_CAR,
                                       COL_TRACK | COL_RAY | COL_DYNAMIC_TRACK | COL_VROAD | COL_CAR);
        m_pDynamicsWorld->addVehicle(car->GetVehicle());

        // Wire up the wheels
        const float wheelRadius{car->vehicleProperties.wheelRadius};
        const btScalar sRestLength {car->vehicleProperties.suspensionRestLength};
        const btVector3 wheelDirectionCS0(0, -1, 0);
        const btVector3 wheelAxleCS(-1, 0, 0);
        // Fronties
        car->GetVehicle()->addWheel(Utils::glmToBullet(car->leftFrontWheelModel.geometry->position), wheelDirectionCS0,
                                    wheelAxleCS, sRestLength, wheelRadius, car->tuning, true);
        car->GetVehicle()->addWheel(Utils::glmToBullet(car->rightFrontWheelModel.geometry->position), wheelDirectionCS0,
                                    wheelAxleCS, sRestLength, wheelRadius, car->tuning, true);
        // Rearies
        car->GetVehicle()->addWheel(Utils::glmToBullet(car->leftRearWheelModel.geometry->position), wheelDirectionCS0,
                                    wheelAxleCS, sRestLength, wheelRadius, car->tuning, false);
        car->GetVehicle()->addWheel(Utils::glmToBullet(car->rightRearWheelModel.geometry->position), wheelDirectionCS0,
                                    wheelAxleCS, sRestLength, wheelRadius, car->tuning, false);

        for (auto wheelIdx = 0; wheelIdx < car->GetVehicle()->getNumWheels(); ++wheelIdx) {
            btWheelInfo &wheel = car->GetVehicle()->getWheelInfo(wheelIdx);
            wheel.m_suspensionStiffness = car->vehicleProperties.suspensionStiffness;
            wheel.m_wheelsDampingRelaxation = car->vehicleProperties.suspensionDamping;
            wheel.m_wheelsDampingCompression = car->vehicleProperties.suspensionCompression;
            wheel.m_frictionSlip = car->vehicleProperties.wheelFriction;
            wheel.m_rollInfluence = car->vehicleProperties.rollInfluence;
        }

        // Add vehicle to active vehicles list so they can be updated on step of physics engine
        m_activeVehicles.push_back(car);
    }

    btDiscreteDynamicsWorld *PhysicsEngine::GetDynamicsWorld() {
        return m_pDynamicsWorld.get();
    }

    PhysicsEngine::~PhysicsEngine() {
        for (auto &car: m_activeVehicles) {
            m_pDynamicsWorld->removeVehicle(car->GetVehicle());
        }
        for (auto &entity: m_track->entities) {
            m_pDynamicsWorld->removeRigidBody(entity->rigidBody.get());
        }
    }
} // namespace OpenNFS
