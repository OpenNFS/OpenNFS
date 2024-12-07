#pragma once

#include <optional>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <BulletCollision/BroadphaseCollision/btBroadphaseInterface.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

#include "../Scene/Track.h"
#include "../Renderer/BulletDebugDrawer.h"
#include "Car.h"

namespace OpenNFS {
    struct WorldRay {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    class PhysicsEngine {
    public:
        explicit PhysicsEngine(const Track &track);
        ~PhysicsEngine();
        void StepSimulation(float time, const std::vector<uint32_t> &racerResidentTrackblockIDs) const;
        void RegisterVehicle(const std::shared_ptr<Car> &car);
        [[nodiscard]] std::optional<Entity *> CheckForPicking(float x, float y, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) const;
        [[nodiscard]] btDiscreteDynamicsWorld *GetDynamicsWorld() const;

        std::shared_ptr<BulletDebugDrawer> debugDrawer;

    private:
        void _GenerateVroadBarriers();

        const Track& m_track;
        std::vector<std::shared_ptr<Car>> m_activeVehicles;

        std::unique_ptr<btBroadphaseInterface> m_pBroadphase;
        std::unique_ptr<btDefaultCollisionConfiguration> m_pCollisionConfiguration;
        std::unique_ptr<btCollisionDispatcher> m_pDispatcher;
        std::unique_ptr<btSequentialImpulseConstraintSolver> m_pSolver;
        std::unique_ptr<btDiscreteDynamicsWorld> m_pDynamicsWorld;
    };
} // namespace OpenNFS
