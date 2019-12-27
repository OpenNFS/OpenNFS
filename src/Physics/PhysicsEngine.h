#pragma once

#include <GL/glew.h>
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

#include <vector>

#include "../Util/Utils.h"
#include "../Loaders/TrackLoader.h"
#include "../Renderer/BulletDebugDrawer.h"
#include "Car.h"



struct WorldRay
{
    glm::vec3 origin;
    glm::vec3 direction;
};

class PhysicsEngine{
public:
    PhysicsEngine();
    ~PhysicsEngine();
    void InitSimulation();
    void StepSimulation(float time);
    void RegisterVehicle(std::shared_ptr<Car> &car);
    void RegisterTrack(const std::shared_ptr<ONFSTrack> &track);
    Entity *CheckForPicking(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, bool *entityTargeted);

    BulletDebugDrawer debugDrawer;
    btDiscreteDynamicsWorld *dynamicsWorld;
private:
    std::shared_ptr<ONFSTrack> currentTrack;
    std::vector<std::shared_ptr<Car>> m_activeVehicles;
    btBroadphaseInterface *m_broadphase;
    btDefaultCollisionConfiguration *collisionConfiguration;
    btCollisionDispatcher *dispatcher;
    btSequentialImpulseConstraintSolver *solver;
};
