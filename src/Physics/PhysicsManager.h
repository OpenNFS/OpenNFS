#pragma once

#include <BulletCollision/BroadphaseCollision/btBroadphaseInterface.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include <vector>

#include "../Renderer/BulletDebugDrawer.h"
#include "../Scene/Track.h"
#include "Car.h"

namespace OpenNFS {
    struct WorldRay {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    class PhysicsManager {
      public:
        explicit PhysicsManager(std::shared_ptr<Track> const &track);
        ~PhysicsManager();
        void StepSimulation(float time, std::vector<uint32_t> const &racerResidentTrackblockIDs) const;
        void RegisterVehicle(std::shared_ptr<Car> const &car);
        [[nodiscard]] std::optional<Entity *> CheckForPicking(double x, double y, glm::mat4 const &viewMatrix,
                                                              glm::mat4 const &projectionMatrix) const;
        [[nodiscard]] btDiscreteDynamicsWorld *GetDynamicsWorld() const;

        std::shared_ptr<BulletDebugDrawer> debugDrawer;

      private:
        std::shared_ptr<Track> const &m_track;
        std::vector<std::shared_ptr<Car>> m_activeVehicles;

        btBroadphaseInterface *m_pBroadphase = nullptr;
        btDefaultCollisionConfiguration *m_pCollisionConfiguration = nullptr;
        btCollisionDispatcher *m_pDispatcher = nullptr;
        btSequentialImpulseConstraintSolver *m_pSolver = nullptr;
        btDiscreteDynamicsWorld *m_pDynamicsWorld = nullptr;
    };
} // namespace OpenNFS
