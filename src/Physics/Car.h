#pragma once

#include "NFS4VehiclePhysics.h"

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
#include "PhysicsModels.h"

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
        // Accel/Brakes (Bullet: Force)
        float gEngineForce{};
        float gBrakingForce{};

        // Accel/Brakes (NFS4: Raw 0 -> 1.0)
        float throttlePedal{};
        float brakePedal{};
        bool handbrake{};
        Gear requestedGear{Gear::NEUTRAL};

        // Steering
        float gVehicleSteering{};
        bool steerRight{};
        bool steerLeft{};

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
        void Update(btDynamicsWorld const *dynamicsWorld, float dt);
        void UpdateMeshesToTransform(btTransform const &trans, bool avoidPhysics = false);
        void SetPosition(glm::vec3 position, glm::quat orientation);
        void ApplyAccelerationForce(bool accelerate, bool reverse);
        void ApplyBrakingForce(bool apply);
        void ApplySteeringRight(bool apply);
        void ApplySteeringLeft(bool apply);
        void ApplyGearChange(bool up, bool down);
        void ApplyHandbrakes(bool apply);
        void ToggleHeadlights();
        void ApplyAbsoluteSteerAngle(float targetAngle);
        [[nodiscard]] float GetCarBodyOrientation() const;

        // Physics Engine registration
        void SetPhysicsVehicle(std::unique_ptr<btRaycastVehicle> &&vehicle);
        void SetRaycaster(std::unique_ptr<btVehicleRaycaster> &&vehicleRayCaster) {
            m_vehicleRayCaster = std::move(vehicleRayCaster);
        }
        [[nodiscard]] NFS4VehiclePhysics *GetNFS4VehiclePhysics() const {
            return m_nfs4VehiclePhysics.get();
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
        PhysicsModel physicsModel{};

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
        void _GenPhysicsShapes();
        void _GenRaycasts(btDynamicsWorld const *dynamicsWorld);
        void _SetModels(std::vector<LibOpenNFS::CarGeometry> &carGeometries);
        void _SetVehicleColour();

        // Base Physics objects for car
        std::unique_ptr<btDefaultMotionState> m_vehicleMotionState{}; // Retrieving vehicle location in world
        std::unique_ptr<btRigidBody> m_carChassis{};
        btAlignedObjectArray<btCollisionShape *> m_collisionShapes;
        std::unique_ptr<btVehicleRaycaster> m_vehicleRayCaster{}; // Wheel simulation
        std::unique_ptr<btRaycastVehicle> m_vehicle{};
        std::unique_ptr<NFS4VehiclePhysics> m_nfs4VehiclePhysics{};
    };
} // namespace OpenNFS
