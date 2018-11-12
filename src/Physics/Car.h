//
// Created by Amrik Sadhra on 18/03/2018.
//

#pragma once

#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <BulletDynamics/Vehicle/btVehicleRaycaster.h>
#include <LinearMath/btDefaultMotionState.h>
#include <btBulletDynamicsCommon.h>

#include "../Enums.h"
#include "../RaceNet/RaceNet.h"
#include "../Scene/CarModel.h"
#include "../Util/Utils.h"

class Car {
  public:
    explicit Car(std::vector<CarModel> car_meshes, NFSVer nfs_version, std::string car_name);
    Car(uint16_t populationID, std::vector<CarModel> car_meshes, NFSVer nfs_version, std::string car_name,
        RaceNet carNet); // Neural Net/GA Training
    Car(std::vector<CarModel> car_meshes, NFSVer nfs_version, std::string car_name,
        GLuint car_textureArrayID); // Multitextured car
    ~Car();
    void setPosition(glm::vec3 position);
    void setNetwork(RaceNet &carNet) { this->carNet = carNet; };
    void update();
    void simulate();
    void update(btDynamicsWorld *dynamicsWorld);
    void resetCar(glm::vec3 reset_position);
    void writeObj(const std::string &path);

    std::string name;
    NFSVer tag;
    uint16_t populationID = -1;
    bool multitexturedCarModel = false;
    glm::vec3 colour;
    // Car Neural Net
    RaceNet carNet;

    btDefaultMotionState *getMotionState() { return vehicleMotionState; }
    btRigidBody *getVehicleRigidBody() { return m_carChassis; }
    btVehicleRaycaster *getRaycaster() { return m_vehicleRayCaster; }
    btRaycastVehicle *getRaycast() { return m_vehicle; }

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
    bool isMultitextured() { return multitexturedCarModel; }
    bool hasPolyFlags() { return car_body_model.hasPolyFlags; }

    float getRotY();

    // Raycasting Data
    static constexpr float castDistance = 1.f;
    static constexpr float castHeightOffset = 0.05f;

    glm::vec3 forwardCastPosition, upCastPosition, rightCastPosition, leftCastPosition;
    float forwardDistance = 0.f;
    float upDistance = 0.f;
    float rightDistance = 0.f;
    float leftDistance = 0.f;

    // Meshes
    std::vector<CarModel> all_models;
    std::vector<CarModel> misc_models;
    CarModel left_front_wheel_model;
    CarModel right_front_wheel_model;
    CarModel left_rear_wheel_model;
    CarModel right_rear_wheel_model;
    CarModel car_body_model;
    // Multitextured Car
    GLuint textureArrayID;

    // Physics
    btRaycastVehicle::btVehicleTuning m_tuning; // Wheel properties
    btVehicleRaycaster *m_vehicleRayCaster;     // Wheel simulation
    btRaycastVehicle *m_vehicle;

    // Vehicle Properties
    float gVehicleSteering;
    float steeringIncrement; // Steering speed
    float steeringClamp;     // Max steering angle

    float gEngineForce;     // force to apply to engine
    float gBreakingForce;   // breaking force
    float maxEngineForce;   // max engine force to apply
    float maxBreakingForce; // max breaking force

    float wheelRadius;
    float wheelWidth;
    btScalar suspensionRestLength;

    // Wheel Properties
    float suspensionStiffness;
    float suspensionDamping;
    float suspensionCompression;
    float wheelFriction;
    // Shifts CoM
    float rollInfluence;

  private:
    void genRaycasts(btDynamicsWorld *dynamicsWorld);
    void setModels(std::vector<CarModel> car_models);

    // Base Physics objects for car
    btDefaultMotionState *vehicleMotionState; // Retrieving vehicle location in world
    btRigidBody *m_carChassis;
    btAlignedObjectArray<btCollisionShape *> m_collisionShapes;

    // Steering state
    bool steerRight;
    bool steerLeft;
    bool isSteering;
};
