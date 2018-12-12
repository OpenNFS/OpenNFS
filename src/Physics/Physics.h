//
// Created by Amrik Sadhra on 05/02/2018.
//

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
#include "../Scene/TrackBlock.h"
#include "../Loaders/trk_loader.h"
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

void ScreenPosToWorldRay(
        int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
        int screenWidth, int screenHeight,  // Window size, in pixels
        glm::mat4 ViewMatrix,               // Camera position and orientation
        glm::mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
        glm::vec3 &out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
        glm::vec3 &out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
);

class Physics{
public:
    Physics();
    ~Physics(){ cleanSimulation(); }
    void initSimulation();
    void stepSimulation(float time);
    void cleanSimulation();
    btDynamicsWorld* getDynamicsWorld() { return dynamicsWorld; }
    void registerVehicle(std::shared_ptr<Car> &car);
    void registerTrack(const std::shared_ptr<ONFSTrack> &track);

    BulletDebugDrawer_DeprecatedOpenGL mydebugdrawer;

    void checkForFrustumIntersect();
    void updateFrustrum(glm::mat4 viewMatrix);
    void destroyGhostObject();
    int numObjects = 0;
    btAlignedObjectArray<btCollisionObject*> m_objectsInFrustum;	// Frustum cull results
private:
    shared_ptr<ONFSTrack> current_track;
    std::vector<std::shared_ptr<Car>> cars;
    /*------- BULLET --------*/
    btBroadphaseInterface *broadphase;
    btDefaultCollisionConfiguration *collisionConfiguration;
    btCollisionDispatcher *dispatcher;
    btSequentialImpulseConstraintSolver *solver;
    btDiscreteDynamicsWorld *dynamicsWorld;
    // Frustum Culling
    btPairCachingGhostObject* m_ghostObject = nullptr;
    btOverlappingPairCallback*	m_ghostPairCallback = nullptr;
    btCollisionShape* buildFrustumShape();
    void buildGhostObject();
};
