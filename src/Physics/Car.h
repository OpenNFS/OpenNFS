#pragma once

#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Vehicle/btVehicleRaycaster.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <btBulletDynamicsCommon.h>

#include "../Scene/Models/CarModel.h"
#include "../Scene/Spotlight.h"
#include "../Util/ImageLoader.h"
#include "../Util/Utils.h"
#include "Common/NFSVersion.h"

// Raycasting Data
enum RayDirection : uint8_t {
    LEFT_RAY          = 0,
    FORWARD_LEFT_RAY  = 8,
    FORWARD_RAY       = 9,
    FORWARD_RIGHT_RAY = 10,
    RIGHT_RAY         = 18,
};

constexpr uint8_t kNumRangefinders = 19;
constexpr float kFarDistance       = 5.f;
constexpr float kAngleBetweenRays  = 10.f;
constexpr float kCastDistance      = 1.f;

enum Wheels : uint8_t { FRONT_LEFT = 0, FRONT_RIGHT, REAR_LEFT, REAR_RIGHT };

struct VehicleProperties {
    float mass;

    // Engine
    float maxEngineForce;   // Max engine force to apply
    float maxBreakingForce; // Max breaking force
    float maxSpeed;         // Max speed before stop applying engine force

    // Steering
    bool absoluteSteer;      // Use absolute steering
    float steeringIncrement; // Steering speed
    float steeringClamp;     // Max steering angle

    // Wheel
    float wheelRadius;
    float wheelWidth;
    float wheelFriction;
    btScalar suspensionRestLength;

    // Suspension
    float suspensionStiffness;
    float suspensionDamping;
    float suspensionCompression;
    float rollInfluence; // Shift CoM

    // Visual
    glm::vec3 colour;
};

struct VehicleState {
    // Engine
    float gEngineForce;   // Force to apply to engine
    float gBreakingForce; // Breaking force

    // Steering
    float gVehicleSteering;
    bool steerRight;
    bool steerLeft;
    bool isSteering;
};

struct RangefinderInfo {
    float rangefinders[kNumRangefinders];
    glm::vec3 castPositions[kNumRangefinders];
    glm::vec3 upCastPosition, downCastPosition;
    float upDistance = 0.f, downDistance = 0.f;
};

struct RenderInfo {
    bool isMultitexturedModel = false;
    GLuint textureID{};      // TGA texture ID
    GLuint textureArrayID{}; // Multitextured texture ID
};

class Car {
public:
    explicit Car(const CarData& carData, NFSVersion nfsVersion, const std::string& carID);
    Car(const CarData& carData, NFSVersion nfsVersion, const std::string& carID, GLuint textureArrayID); // Multitextured car
    ~Car();
    void Update(btDynamicsWorld* dynamicsWorld);
    void SetPosition(glm::vec3 position, glm::quat orientation);
    void ApplyAccelerationForce(bool accelerate, bool reverse);
    void ApplyBrakingForce(bool apply);
    void ApplySteeringRight(bool apply);
    void ApplySteeringLeft(bool apply);
    void ApplyAbsoluteSteerAngle(float targetAngle);
    float GetCarBodyOrientation();

    // Physics Engine registration
    void SetVehicle(btRaycastVehicle* vehicle) {
        m_vehicle = vehicle;
    }
    void SetRaycaster(btVehicleRaycaster* vehicleRayCaster) {
        m_vehicleRayCaster = vehicleRayCaster;
    }
    btRigidBody* GetVehicleRigidBody() {
        return m_carChassis;
    }
    btVehicleRaycaster* GetRaycaster() {
        return m_vehicleRayCaster;
    }
    btRaycastVehicle* GetVehicle() {
        return m_vehicle;
    }

    std::string name;
    std::string id;
    NFSVersion tag;
    CarData assetData;

    // Car configuration data
    VehicleProperties vehicleProperties{};
    VehicleState vehicleState{};
    RangefinderInfo rangefinderInfo{};
    RenderInfo renderInfo{};

    // Meshes and Headlights
    Spotlight leftHeadlight{};
    Spotlight rightHeadlight{};
    std::vector<CarModel> miscModels;
    CarModel leftFrontWheelModel;
    CarModel rightFrontWheelModel;
    CarModel leftRearWheelModel;
    CarModel rightRearWheelModel;
    CarModel carBodyModel;

    // Wheel properties
    btRaycastVehicle::btVehicleTuning tuning;

private:
    void _UpdateMeshesToMatchPhysics();
    void _ApplyInputs();
    void _LoadTextures();
    void _GenPhysicsModel();
    void _GenRaycasts(btDynamicsWorld* dynamicsWorld);
    void _SetModels(std::vector<CarModel> carModels);
    void _SetVehicleProperties();

    // Base Physics objects for car
    btDefaultMotionState* m_vehicleMotionState{}; // Retrieving vehicle location in world
    btRigidBody* m_carChassis{};
    btAlignedObjectArray<btCollisionShape*> m_collisionShapes;
    btVehicleRaycaster* m_vehicleRayCaster{}; // Wheel simulation
    btRaycastVehicle* m_vehicle{};
};
