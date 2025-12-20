#include "PhysicsManager.h"

#include <ranges>

#include "CollisionMasks.h"

namespace OpenNFS {
    WorldRay ScreenPosToWorldRay(
        float mouseX, float mouseY, uint32_t screenWidth, uint32_t screenHeight, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix) {
        // The ray Start and End positions, in Normalized Device Coordinates
        glm::vec4 lRayStart_NDC((mouseX / static_cast<float>(screenWidth) - 0.5f) * 2.0f,
                                (mouseY / static_cast<float>(screenHeight) - 0.5f) * 2.0f,
                                -1.0, // The near plane maps to Z=-1 in Normalized Device Coordinates
                                1.0f);
        glm::vec4 lRayEnd_NDC((mouseX / static_cast<float>(screenWidth) - 0.5f) * 2.0f,
                              (mouseY / static_cast<float>(screenHeight) - 0.5f) * 2.0f, 0.0, 1.0f);

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

    PhysicsManager::PhysicsManager(Track const &track) : debugDrawer(std::make_shared<BulletDebugDrawer>()), m_track(track) {
        m_pBroadphase = std::make_unique<btDbvtBroadphase>();
        // Set up the collision configuration and dispatcher
        m_pCollisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
        m_pDispatcher = std::make_unique<btCollisionDispatcher>(m_pCollisionConfiguration.get());
        // The actual physics solver
        m_pSolver = std::make_unique<btSequentialImpulseConstraintSolver>();
        // The world.
        m_pDynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(m_pDispatcher.get(), m_pBroadphase.get(), m_pSolver.get(),
                                                                     m_pCollisionConfiguration.get());
        m_pDynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
        m_pDynamicsWorld->setDebugDrawer(debugDrawer.get());

        // Register the Track
        for (auto const &trackBlockEntities : track.perTrackblockEntities) {
            for (auto const &entity : trackBlockEntities) {
                int collisionMask = COL_RAY | COL_CAR;
                if (!entity->collidable) {
                    continue;
                }
                if (entity->dynamic) {
                    collisionMask |= COL_TRACK;
                    // Move Rigid body to correct place in world
                    btTransform initialTransform = Utils::MakeTransform(entity->initialPosition, entity->orientation);
                    entity->rigidBody->setWorldTransform(initialTransform);
                    m_pDynamicsWorld->addRigidBody(entity->rigidBody.get(), COL_DYNAMIC_TRACK, collisionMask);
                } else {
                    m_pDynamicsWorld->addRigidBody(entity->rigidBody.get(), COL_TRACK, collisionMask);
                }
            }
        }
    }

    void PhysicsManager::StepSimulation(float const time, std::vector<uint32_t> const &racerResidentTrackblockIDs) const {
        m_pDynamicsWorld->stepSimulation(time, 100);

        for (auto const &car : m_activeVehicles) {
            car->Update(m_pDynamicsWorld.get());
        }

        auto const racerResidentTrackblockEntities{
            racerResidentTrackblockIDs |
            std::views::transform([&](uint32_t const index) -> auto & { return m_track.perTrackblockEntities[index]; }) | std::views::join};

        for (auto const &entity : racerResidentTrackblockEntities) {
            entity->Update();
        }
    }

    std::optional<Entity *> PhysicsManager::CheckForPicking(float const x,
                                                            float const y,
                                                            glm::mat4 const &viewMatrix,
                                                            glm::mat4 const &projectionMatrix) const {
        auto const [origin, direction]{
            ScreenPosToWorldRay(x, y, Config::get().windowSizeX, Config::get().windowSizeY, viewMatrix, projectionMatrix)};
        glm::vec3 const outEnd{origin + direction * 1000.0f};

        btCollisionWorld::ClosestRayResultCallback rayCallback(Utils::glmToBullet(origin), Utils::glmToBullet(outEnd));
        rayCallback.m_collisionFilterMask = COL_CAR | COL_TRACK | COL_DYNAMIC_TRACK;

        m_pDynamicsWorld->rayTest(Utils::glmToBullet(origin), Utils::glmToBullet(outEnd), rayCallback);

        return rayCallback.hasHit() ? static_cast<Entity *>(rayCallback.m_collisionObject->getUserPointer())
                                    : std::optional<Entity *>(std::nullopt);
    }

    void PhysicsManager::RegisterVehicle(std::shared_ptr<Car> const &car) {
        car->SetRaycaster(std::make_unique<btDefaultVehicleRaycaster>(m_pDynamicsWorld.get()));
        car->SetVehicle(std::make_unique<btRaycastVehicle>(car->tuning, car->GetVehicleRigidBody(), car->GetRaycaster()));
        car->GetVehicle()->setCoordinateSystem(0, 1, 2);

        m_pDynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(car->GetVehicleRigidBody()->getBroadphaseHandle(),
                                                                                          m_pDynamicsWorld->getDispatcher());
        m_pDynamicsWorld->addRigidBody(car->GetVehicleRigidBody(), COL_CAR, COL_TRACK | COL_RAY | COL_DYNAMIC_TRACK | COL_CAR);
        m_pDynamicsWorld->addVehicle(car->GetVehicle());

        // Wire up the wheels
        float const wheelRadius{car->assetData.physicsData.wheelRadius};
        btScalar const sRestLength{car->assetData.physicsData.suspensionRestLength};
        btVector3 const wheelDirectionCS0(0, -1, 0);
        btVector3 const wheelAxleCS(-1, 0, 0);
        // Fronties
        car->GetVehicle()->addWheel(Utils::glmToBullet(car->leftFrontWheelModel.position), wheelDirectionCS0, wheelAxleCS, sRestLength,
                                    wheelRadius, car->tuning, true);
        car->GetVehicle()->addWheel(Utils::glmToBullet(car->rightFrontWheelModel.position), wheelDirectionCS0, wheelAxleCS, sRestLength,
                                    wheelRadius, car->tuning, true);
        // Rearies
        car->GetVehicle()->addWheel(Utils::glmToBullet(car->leftRearWheelModel.position), wheelDirectionCS0, wheelAxleCS, sRestLength,
                                    wheelRadius, car->tuning, false);
        car->GetVehicle()->addWheel(Utils::glmToBullet(car->rightRearWheelModel.position), wheelDirectionCS0, wheelAxleCS, sRestLength,
                                    wheelRadius, car->tuning, false);

        for (auto wheelIdx = 0; wheelIdx < car->GetVehicle()->getNumWheels(); ++wheelIdx) {
            btWheelInfo &wheel = car->GetVehicle()->getWheelInfo(wheelIdx);
            wheel.m_suspensionStiffness = car->assetData.physicsData.suspensionStiffness;
            wheel.m_wheelsDampingRelaxation = car->assetData.physicsData.suspensionDamping;
            wheel.m_wheelsDampingCompression = car->assetData.physicsData.suspensionCompression;
            wheel.m_frictionSlip = car->assetData.physicsData.wheelFriction;
            wheel.m_rollInfluence = car->assetData.physicsData.rollInfluence;
        }

        // Add vehicle to active vehicles list so they can be updated on step of physics engine
        m_activeVehicles.push_back(car);
    }

    btDiscreteDynamicsWorld *PhysicsManager::GetDynamicsWorld() const {
        return m_pDynamicsWorld.get();
    }

    PhysicsManager::~PhysicsManager() {
        for (auto const &car : m_activeVehicles) {
            m_pDynamicsWorld->removeVehicle(car->GetVehicle());
        }
        for (auto const &trackBlockEntities : m_track.perTrackblockEntities) {
            for (auto const &entity : trackBlockEntities) {
                m_pDynamicsWorld->removeRigidBody(entity->rigidBody.get());
            }
        }
    }
} // namespace OpenNFS
