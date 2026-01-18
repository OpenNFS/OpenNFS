#pragma once

#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <BulletDynamics/Vehicle/btVehicleRaycaster.h>
#include <Entities/Car.h>
#include <LinearMath/btDefaultMotionState.h>
#include <btBulletDynamicsCommon.h>

#include "../Renderer/Models/GLCarModel.h"
#include "../Scene/GlobalLight.h"
#include "../Scene/Spotlight.h"
#include "../Util/ImageLoader.h"
#include "../Util/Utils.h"

namespace OpenNFS {
    // Raycasting Data
    enum RayDirection : uint8_t {
        LEFT_RAY = 0,
        FORWARD_LEFT_RAY = 8,
        FORWARD_RAY = 9,
        FORWARD_RIGHT_RAY = 10,
        RIGHT_RAY = 18,
    };

    constexpr uint8_t kNumRangefinders = 19;
    constexpr float kFarDistance = 5.f;
    constexpr float kAngleBetweenRays = 10.f;
    constexpr float kCastDistance = 1.f;

    enum Wheels : uint8_t {
        FRONT_LEFT = 0,
        FRONT_RIGHT,
        REAR_LEFT,
        REAR_RIGHT
    };

    struct VehicleState {
        // Engine
        float gEngineForce;
        float gBreakingForce;

        // Steering
        float gVehicleSteering;
        bool steerRight;
        bool steerLeft;
        bool isSteering;
        bool isBraking;

        // Accessories
        bool headlightsActive;
        bool taillightsActive;

        // Meta
        glm::vec4 colour;
        glm::vec4 colourSecondary;
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
        explicit Car(LibOpenNFS::Car car);
        Car(LibOpenNFS::Car const &car, GLuint textureArrayID); // Multitextured car
        ~Car();
        void Update(btDynamicsWorld const *dynamicsWorld);
        void UpdateMeshesToTransform(btTransform const &trans, bool avoidPhysics = false);
        void SetPosition(glm::vec3 position, glm::quat orientation);
        void ApplyAccelerationForce(bool accelerate, bool reverse);
        void ApplyBrakingForce(bool apply);
        void ApplySteeringRight(bool apply);
        void ApplySteeringLeft(bool apply);
        void ToggleHeadlights();
        void ApplyAbsoluteSteerAngle(float targetAngle);
        [[nodiscard]] float GetCarBodyOrientation() const;

        // Physics Engine registration
        void SetVehicle(std::unique_ptr<btRaycastVehicle> &&vehicle) {
            m_vehicle = std::move(vehicle);
        }
        void SetRaycaster(std::unique_ptr<btVehicleRaycaster> &&vehicleRayCaster) {
            m_vehicleRayCaster = std::move(vehicleRayCaster);
        }
        [[nodiscard]] btRigidBody *GetVehicleRigidBody() const {
            return m_carChassis.get();
        }
        [[nodiscard]] btVehicleRaycaster *GetRaycaster() const {
            return m_vehicleRayCaster.get();
        }
        [[nodiscard]] btRaycastVehicle *GetVehicle() const {
            return m_vehicle.get();
        }

        LibOpenNFS::Car assetData;

        // Car configuration data
        VehicleState vehicleState{};
        RangefinderInfo rangefinderInfo{};
        RenderInfo renderInfo{};

        // Meshes and Headlights
        Spotlight leftHeadLight;
        Spotlight rightHeadLight;
        GlobalLight leftTailLight;
        GlobalLight rightTailLight;
        std::vector<GLCarModel> miscModels;
        GLCarModel leftFrontWheelModel;
        GLCarModel rightFrontWheelModel;
        GLCarModel leftRearWheelModel;
        GLCarModel rightRearWheelModel;
        GLCarModel carBodyModel;

        // Wheel properties
        btRaycastVehicle::btVehicleTuning tuning;

      private:
        void _ApplyInputs();
        void _LoadTextures();
        void _GenPhysicsModel();
        void _GenRaycasts(btDynamicsWorld const *dynamicsWorld);
        void _SetModels(std::vector<LibOpenNFS::CarGeometry> &carGeometries);
        void _SetVehicleState();

        // Base Physics objects for car
        std::unique_ptr<btDefaultMotionState> m_vehicleMotionState{}; // Retrieving vehicle location in world
        std::unique_ptr<btRigidBody> m_carChassis{};
        btAlignedObjectArray<btCollisionShape *> m_collisionShapes;
        std::unique_ptr<btVehicleRaycaster> m_vehicleRayCaster{}; // Wheel simulation
        std::unique_ptr<btRaycastVehicle> m_vehicle{};
    };
} // namespace OpenNFS
