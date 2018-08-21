//
// Created by Amrik Sadhra on 18/03/2018.
//

#pragma once

#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Vehicle/btVehicleRaycaster.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <btBulletDynamicsCommon.h>

#include "../Scene/CarModel.h"
#include "../Util/Utils.h"
#include "../Enums.h"

class Car {
public:
    explicit Car(std::vector<CarModel> car_meshes, NFSVer nfs_version, std::string car_name);
    Car(std::vector<CarModel> car_meshes, NFSVer nfs_version, std::string car_name, std::map<unsigned int, GLuint> texture_gl_mappings); // Multitextured car
    ~Car();
    void setPosition(glm::vec3 position);
    void update();
    void resetCar(glm::vec3 reset_position);
    void writeObj(const std::string &path);

    std::string name;
    NFSVer tag;

    btDefaultMotionState* getMotionState() { return vehicleMotionState; }
    btRigidBody* getVehicleRigidBody() { return m_carChassis; }
    btVehicleRaycaster* getRaycaster() { return m_vehicleRayCaster; }
    btRaycastVehicle*	getRaycast() { return m_vehicle; }

    void applyAccelerationForce(bool accelerate, bool reverse);
    void applyBrakingForce(bool apply);
    void applySteeringRight(bool apply);
    void applySteeringLeft(bool apply);

    float getWheelRadius() { return wheelRadius; }
    float getWheelWidth() { return wheelWidth; }
    btScalar getSuspensionRestLength() { return suspensionRestLength; }
    float getSuspensionStiffness() { return suspensionStiffness; }
    float getSuspensionDamping() { return suspensionDamping; }
    float getSuspensionCompression() { return suspensionCompression; }
    float getWheelFriction() { return wheelFriction; }
    float getRollInfluence() { return rollInfluence; }
    bool isMultitextured() {return (bool) car_body_model.texture_ids.size(); }

    double getRotY();

    // Meshes
    std::vector<CarModel> misc_models;
    CarModel left_front_wheel_model;
    CarModel right_front_wheel_model;
    CarModel left_rear_wheel_model;
    CarModel right_rear_wheel_model;
    CarModel car_body_model;
    // Multitextured Car
    std::map<unsigned int, GLuint> car_texture_gl_mappings;

    // Physics
    btRaycastVehicle::btVehicleTuning m_tuning; // Wheel properties
    btVehicleRaycaster* m_vehicleRayCaster;     // Wheel simulation
    btRaycastVehicle* m_vehicle;

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
private:
    void setModels(std::vector<CarModel> car_models);

    // Base Physics objects for car
    btDefaultMotionState* vehicleMotionState;   // Retrieving vehicle location in world
    btRigidBody* m_carChassis;
    btAlignedObjectArray<btCollisionShape*> m_collisionShapes;

    // Steering state
    bool steerRight;
    bool steerLeft;
    bool isSteering;
};
