#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/vec3.hpp>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/BroadphaseCollision/btBroadphaseInterface.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include "../Util/Utils.h"
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
        PhysicsEngine();
        ~PhysicsEngine();
        void StepSimulation(float time, const std::vector<uint32_t> &racerResidentTrackblockIDs);
        void RegisterVehicle(const std::shared_ptr<Car> &car);
        void RegisterTrack(const std::shared_ptr<OpenNFS::Track> &track);
        std::optional<Entity *> CheckForPicking(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);
        btDiscreteDynamicsWorld *GetDynamicsWorld();

        std::shared_ptr<BulletDebugDrawer> debugDrawer;

    private:
        void _GenerateVroadBarriers();

        std::shared_ptr<OpenNFS::Track> m_track;
        std::vector<std::shared_ptr<Car>> m_activeVehicles;

        std::unique_ptr<btBroadphaseInterface> m_pBroadphase;
        std::unique_ptr<btDefaultCollisionConfiguration> m_pCollisionConfiguration;
        std::unique_ptr<btCollisionDispatcher> m_pDispatcher;
        std::unique_ptr<btSequentialImpulseConstraintSolver> m_pSolver;
        std::unique_ptr<btDiscreteDynamicsWorld> m_pDynamicsWorld;
    };

} // namespace OpenNFS
