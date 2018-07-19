//
// Created by Amrik Sadhra on 05/02/2018.
//

#pragma once

#include <glm/vec3.hpp>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <vector>
#include <btBulletDynamicsCommon.h>
#include "../Scene/TrackBlock.h"
#include <BulletCollision/BroadphaseCollision/btBroadphaseInterface.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <GL/glew.h>
#include <glm/detail/type_mat.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include "../Util/Utils.h"
#include "Car.h"

class BulletDebugDrawer_DeprecatedOpenGL : public btIDebugDraw {
public:
    void SetMatrices(glm::mat4 pViewMatrix, glm::mat4 pProjectionMatrix) {
        glUseProgram(0); // Use Fixed-function pipeline (no shaders)
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(&pViewMatrix[0][0]);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(&pProjectionMatrix[0][0]);
    }

    virtual void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {
        glColor3f(color.x(), color.y(), color.z());
        glBegin(GL_LINES);
        glVertex3f(from.x(), from.y(), from.z());
        glVertex3f(to.x(), to.y(), to.z());
        glEnd();
    }

    virtual void drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) {}

    virtual void reportErrorWarning(const char *) {}

    virtual void draw3dText(const btVector3 &, const char *) {}

    virtual void setDebugMode(int p) {
        m = p;
    }

    int getDebugMode(void) const { return 3; }

    int m;
};

class Physics{
public:
    Physics();
    ~Physics(){ cleanSimulation(); }
    void initSimulation();
    void stepSimulation(float time);
    void cleanSimulation();
    btDynamicsWorld* getDynamicsWorld() { return dynamicsWorld; }
    void registerVehicle(std::shared_ptr<Car> car);
    void registerTrack(const std::vector<TrackBlock> &track_blocks);

    BulletDebugDrawer_DeprecatedOpenGL mydebugdrawer;

private:
    std::vector<std::shared_ptr<Car>> cars;
    /*------- BULLET --------*/
    btBroadphaseInterface *broadphase;
    btDefaultCollisionConfiguration *collisionConfiguration;
    btCollisionDispatcher *dispatcher;
    btSequentialImpulseConstraintSolver *solver;
    btDiscreteDynamicsWorld *dynamicsWorld;
    btRigidBody* groundRigidBody;
    btTriangleMesh trackMesh;
};
