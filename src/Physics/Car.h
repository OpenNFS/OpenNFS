//
// Created by Amrik on 18/03/2018.
//

#pragma once

#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Vehicle/btVehicleRaycaster.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <btBulletDynamicsCommon.h>

#include "../Loaders/nfs4_loader.h"
#include "../Loaders/nfs3_loader.h"
#include "../Scene/CarModel.h"
#include "../Util/Utils.h"

class Car {
public:
    Car(NFS3_Loader loader);
    ~Car();
    void update();
    void resetCar();

    btDefaultMotionState* getMotionState() { return vehicleMotionState; }
    btRigidBody* getVehicleRigidBody() { return m_carChassis; }
    btVehicleRaycaster* getRaycaster() { return m_vehicleRayCaster; }
    btRaycastVehicle*	getRaycast() { return m_vehicle; }

    void applyAccelerationForce(bool apply);
    void applyBrakingForce(bool apply);
    void applySteeringRight(bool apply);
    void applySteeringLeft(bool apply);
    void toggleReverse();

    float getWheelRadius() { return wheelRadius; }
    float getWheelWidth() { return wheelWidth; }
    btScalar getSuspensionRestLength() { return suspensionRestLength; }
    float getSuspensionStiffness() { return suspensionStiffness; }
    float getSuspensionDamping() { return suspensionDamping; }
    float getSuspensionCompression() { return suspensionCompression; }
    float getWheelFriction() { return wheelFriction; }
    float getRollInfluence() { return rollInfluence; }

    // Meshes
    std::vector<CarModel> car_models;
    // Physics
    btRaycastVehicle::btVehicleTuning m_tuning; // Wheel properties
    btVehicleRaycaster* m_vehicleRayCaster;     // Wheel simulation
    btRaycastVehicle* m_vehicle;

private:
    // Base Physics objects for car
    btDefaultMotionState* vehicleMotionState;   // Retrieving vehicle location in world
    btRigidBody* m_carChassis;
    btAlignedObjectArray<btCollisionShape*> m_collisionShapes;

    // Vehicle Properties
    float	gVehicleSteering;
    float	steeringIncrement;   // Steering speed
    float	steeringClamp;       // Max steering angle

    float	gEngineForce;        // force to apply to engine
    float	gBreakingForce;      // breaking force
    float	maxEngineForce;      // max engine force to apply
    float	maxBreakingForce;    // max breaking force

    float	wheelRadius;
    float	wheelWidth;
    btScalar suspensionRestLength;

    // Wheel Properties
    float	suspensionStiffness;
    float	suspensionDamping;
    float	suspensionCompression;
    float	wheelFriction;
    // Shifts CoM
    float	rollInfluence;
    // Steering state
    bool steerRight;
    bool steerLeft;
    bool isSteering;
    bool isReverse;

};
