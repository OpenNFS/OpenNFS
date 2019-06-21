#pragma once

#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Vehicle/btVehicleRaycaster.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <btBulletDynamicsCommon.h>

#include "../RaceNet/RaceNet.h"
#include "../Scene/Spotlight.h"
#include "../Scene/CarModel.h"
#include "../Util/ImageLoader.h"
#include "../Util/Utils.h"
#include "../Enums.h"

class Car {
public:
    explicit Car(CarData carData, NFSVer nfs_version, std::string carID);
    Car(CarData carData, NFSVer nfs_version, std::string carID, GLuint car_textureArrayID); // Multitextured car
    ~Car();
    void setPosition(glm::vec3 position, glm::quat orientation);
    void update();
    void update(btDynamicsWorld* dynamicsWorld);
    void resetCar(glm::vec3 reset_position, glm::quat reset_orientation);
    void writeObj(const std::string &path, std::vector<CarModel> modelsToExport);

    std::string name;
    std::string id;
    NFSVer tag;
    bool multitexturedCarModel = false;
    glm::vec3 colour;
    CarData data;

    btDefaultMotionState* getMotionState() { return vehicleMotionState; }
    btRigidBody* getVehicleRigidBody() { return m_carChassis; }
    btVehicleRaycaster* getRaycaster() { return m_vehicleRayCaster; }
    btRaycastVehicle*	getRaycast() { return m_vehicle; }

    void applyAccelerationForce(bool accelerate, bool reverse);
    void applyBrakingForce(bool apply);
    void applySteeringRight(bool apply);
    void applySteeringLeft(bool apply);
    void applyAbsoluteSteerAngle(float targetAngle);

    float getWheelRadius() { return wheelRadius; }
    float getWheelWidth() { return wheelWidth; }
    btScalar getSuspensionRestLength() { return suspensionRestLength; }
    float getSuspensionStiffness() { return suspensionStiffness; }
    float getSuspensionDamping() { return suspensionDamping; }
    float getSuspensionCompression() { return suspensionCompression; }
    float getWheelFriction() { return wheelFriction; }
    float getRollInfluence() { return rollInfluence; }
    bool isMultitextured() {return multitexturedCarModel; }
    bool hasPolyFlags() {return carBodyModel.hasPolyFlags; }

    float getRotY();

    // Raycasting Data
    static constexpr uint8_t LEFT_RAY = 0;
    static constexpr uint8_t FORWARD_LEFT_RAY = 8;
    static constexpr uint8_t FORWARD_RAY = 9;
    static constexpr uint8_t FORWARD_RIGHT_RAY = 10;
    static constexpr uint8_t RIGHT_RAY = 18;

    static constexpr uint8_t kNumRangefinders = 19;
    static constexpr float kFarDistance = 5.f;
    static constexpr float kAngleBetweenRays = 10.f;
    static constexpr float kCastDistance = 1.f;

    float rangefinders[kNumRangefinders];
    glm::vec3 castPositions[kNumRangefinders];
    glm::vec3 upCastPosition, downCastPosition;
    float upDistance = 0.f, downDistance = 0.f;

    // Headlights
    Spotlight leftHeadlight;
    Spotlight rightHeadlight;
    // Meshes
    std::vector<CarModel> miscModels;
    CarModel leftFrontWheelModel;
    CarModel rightFrontWheelModel;
    CarModel leftRearWheelModel;
    CarModel rightRearWheelModel;
    CarModel carBodyModel;

    // GL ID's
    // TGA Car
    GLuint textureID;
    // Multitextured Car
    GLuint textureArrayID;

    // Physics
    btRaycastVehicle::btVehicleTuning m_tuning; // Wheel properties
    btVehicleRaycaster* m_vehicleRayCaster;     // Wheel simulation
    btRaycastVehicle* m_vehicle;

    // Vehicle Properties
    float	gVehicleSteering;
    float	steeringIncrement;   // Steering speed
    float	steeringClamp;       // Max steering angle
    bool    absoluteSteer;

    float   maxSpeed;            // Max speed before stop applying engine force
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
    void genRaycasts(btDynamicsWorld* dynamicsWorld);
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
