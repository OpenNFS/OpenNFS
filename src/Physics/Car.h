#pragma once

#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Vehicle/btVehicleRaycaster.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <btBulletDynamicsCommon.h>
#include <Entities/Car.h>

#include "../Renderer/Models/GLCarModel.h"
#include "../Scene/Spotlight.h"
#include "../Util/ImageLoader.h"
#include "../Util/Utils.h"

namespace OpenNFS {
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
        explicit Car(LibOpenNFS::Car  car);
        Car(const LibOpenNFS::Car& car, GLuint textureArrayID); // Multitextured car
        ~Car();
        void Update(btDynamicsWorld* dynamicsWorld);
        void SetPosition(glm::vec3 position, glm::quat orientation);
        void ApplyAccelerationForce(bool accelerate, bool reverse);
        void ApplyBrakingForce(bool apply);
        void ApplySteeringRight(bool apply);
        void ApplySteeringLeft(bool apply);
        void ApplyAbsoluteSteerAngle(float targetAngle);
        [[nodiscard]] float GetCarBodyOrientation() const;

        // Physics Engine registration
        void SetVehicle(std::unique_ptr<btRaycastVehicle>&& vehicle) {
            m_vehicle = std::move(vehicle);
        }
        void SetRaycaster(std::unique_ptr<btVehicleRaycaster>&& vehicleRayCaster) {
            m_vehicleRayCaster = std::move(vehicleRayCaster);
        }
        [[nodiscard]] btRigidBody* GetVehicleRigidBody() const {
            return m_carChassis.get();
        }
        [[nodiscard]] btVehicleRaycaster* GetRaycaster() const {
            return m_vehicleRayCaster.get();
        }
        [[nodiscard]] btRaycastVehicle* GetVehicle() const {
            return m_vehicle.get();
        }

        LibOpenNFS::Car assetData;

        // Car configuration data
        VehicleProperties vehicleProperties{};
        VehicleState vehicleState{};
        RangefinderInfo rangefinderInfo{};
        RenderInfo renderInfo{};

        // Meshes and Headlights
        Spotlight leftHeadlight{};
        Spotlight rightHeadlight{};
        std::vector<GLCarModel> miscModels;
        GLCarModel leftFrontWheelModel;
        GLCarModel rightFrontWheelModel;
        GLCarModel leftRearWheelModel;
        GLCarModel rightRearWheelModel;
        GLCarModel carBodyModel;

        // Wheel properties
        btRaycastVehicle::btVehicleTuning tuning;

    private:
        void _UpdateMeshesToMatchPhysics();
        void _ApplyInputs();
        void _LoadTextures();
        void _GenPhysicsModel();
        void _GenRaycasts(btDynamicsWorld* dynamicsWorld);
        void _SetModels(std::vector<LibOpenNFS::CarGeometry>& carGeometries);
        void _SetVehicleProperties();

        // Base Physics objects for car
        std::unique_ptr<btDefaultMotionState> m_vehicleMotionState{}; // Retrieving vehicle location in world
        std::unique_ptr<btRigidBody> m_carChassis{};
        btAlignedObjectArray<btCollisionShape*> m_collisionShapes;
        std::unique_ptr<btVehicleRaycaster> m_vehicleRayCaster{}; // Wheel simulation
        std::unique_ptr<btRaycastVehicle> m_vehicle{};
    };
} // namespace OpenNFS
