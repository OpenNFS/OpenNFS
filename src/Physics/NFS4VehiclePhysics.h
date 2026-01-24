#pragma once

#include "RaycastVehicle.h"

#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <Entities/Car.h>
#include <array>
#include <btBulletDynamicsCommon.h>
#include <vector>

/*
 * NFS4 PC Vehicle physics are courtesy of Bluv (e-rk).
 * This class is a direct port to Bullet C++ from Velocity, a Godot re-implementation of High Stakes.
 * https://github.com/e-rk/velocity/blob/master/core/car/handling_model_re.gd
 * The implementation is still WIP within ONFS and must be opted into with the '--hs' flag
 */
namespace OpenNFS {
    // Matches NFS4's gear enum
    enum class Gear : int {
        REVERSE = 0,
        NEUTRAL = 1,
        GEAR_1 = 2,
        GEAR_2 = 3,
        GEAR_3 = 4,
        GEAR_4 = 5,
        GEAR_5 = 6,
        GEAR_6 = 7
    };

    enum class Weather : int {
        DRY = 0,
        RAIN = 1,
        SNOW = 2
    };

    enum class WheelPosition : int {
        FRONT_LEFT = 0,
        FRONT_RIGHT = 1,
        REAR_LEFT = 2,
        REAR_RIGHT = 3
    };

    // NFS4 Performance data (internal representation built from Car::PhysicsData)
    struct NFS4PerformanceData {
        std::array<float, 21> torqueCurve{}; // Torque at 500 RPM intervals
        std::array<float, 8> gearVelocityToRPM{};
        std::array<float, 8> gearEfficiency{};
        std::array<float, 8> shiftBlipRPM{};
        std::array<float, 8> brakeBlipRPM{};
        std::array<uint8_t, 8> brakeIncreasingCurve{};
        std::array<uint8_t, 8> brakeDecreasingCurve{};

        float mass{1750.f};
        float maxVelocity{80.f};
        float engineMinRPM{1000.f};
        float engineRedlineRPM{7000.f};
        float gasOffFactor{0.4f};
        float lateralGripMultiplier{3.5f};
        float frontGripBias{0.5f};
        float frontDriveRatio{0.f};
        float frontBrakeBias{0.6f};
        float understeerGradient{1.f};
        float minimumSteeringAcceleration{18.f};
        float turningCircleRadius{11.f};
        float downforceMult{0.002f};
        float gTransferFactor{0.5f};
        float turnInRamp{16.f};
        float turnOutRamp{32.f};
        float maxBrakingDeceleration{10.f};
        int gearShiftDelay{6};
        int maxGear{7};
        bool hasSpoiler{true};
        bool hasABS{false};

        // Build NFS4PerformanceData from Car::PhysicsData
        static NFS4PerformanceData FromPhysicsData(LibOpenNFS::Car::PhysicsData const &physicsData);
    };

    // Per-wheel computed data
    struct WheelData {
        WheelPosition type;
        float grip;
        float traction;
        float downforce;
        int roadSurface;
    };

    // Debug toggles for individual physics features
    struct NFS4PhysicsToggles {
        bool enableTurningCircle{true};
        bool enableLateralDamping{true};
        bool enableNeutralGearDecel{true};
        bool enableNearStopDecel{true};
        bool enablePreventSideways{true};
        bool enableAirborneDrag{true};
        bool enableLimitAngularVelocity{true};
        bool enableAdjustToRoad{true};
        bool enablePreventSinking{true};
        bool enableDownforce{true};
        bool enableGoAirborne{true};
        bool enableWheelForces{true};
        bool enableTractionModel{true};
    };

    // Vehicle state
    struct NFS4VehicleState {
        // Inputs
        float throttleInput;
        float brakeInput;
        float steerInput;
        bool handbrakeInput;
        Gear requestedGear;

        // Processed inputs (ramped)
        float throttle;
        float brake;
        float currentSteering; // -128 to 128 range like NFS4

        // Engine state
        float rpm;
        Gear gear;
        int gearShiftCounter;
        bool shiftedDown;

        // Physics state
        float slipAngle;
        float speedXZ;
        float gTransfer;
        int handbrakeAccumulator;
        bool lostGrip;
        bool hasContactWithGround;
        float distanceAboveGround;
        bool unknownBool; // Used in gTransferDamp - affects grip when true

        // Computed steering angle (radians)
        float steeringAngle;

        // Computed force from traction model
        float tractionForce;

        Weather weather;

        // Road basis for ground alignment (computed from wheel raycasts)
        btMatrix3x3 basisToRoad;
        btMatrix3x3 basisToRoadNext; // Interpolated for smoother adjustment
    };

    // Debug data for visualization
    struct NFS4DebugData {
        // Per-wheel debug info
        std::array<WheelData, 4> wheels;
        std::array<btVector3, 4> wheelForces;
        std::array<btVector3, 4> wheelWorldPositions;
        std::array<bool, 4> wheelInContact;         // Per-wheel ground contact
        std::array<float, 4> wheelSuspensionLength; // Current suspension compression

        // Total forces applied this frame
        btVector3 totalForce;
        btVector3 totalTorque;

        // Velocity vectors
        btVector3 localVelocity;
        btVector3 localAngularVelocity;

        // Ground orientation
        btVector3 orientationToGround;

        // Computed values
        float drag;
        float torque;
        float slipAngleFactor;
        float tireFactor;
        float angularVelocityFactor;

        float turningCircleAngularDamp;
        float lateralVelocityDamp;
        float nearStopDecelFactor;
        float airborneDownforce;
        bool preventedSideways;
        bool appliedNeutralDecel;
        bool appliedNearStopDecel;
        btVector3 roadAdjustmentAngVel;

        // Rolling history for graphs (circular buffers)
        static constexpr size_t HISTORY_SIZE = 256;
        std::array<float, HISTORY_SIZE> rpmHistory{};
        std::array<float, HISTORY_SIZE> speedHistory{};
        std::array<float, HISTORY_SIZE> throttleHistory{};
        std::array<float, HISTORY_SIZE> brakeHistory{};
        std::array<float, HISTORY_SIZE> steeringHistory{};
        std::array<float, HISTORY_SIZE> slipAngleHistory{};
        std::array<float, HISTORY_SIZE> tractionForceHistory{};
        std::array<float, HISTORY_SIZE> gTransferHistory{};
        size_t historyIndex{0};
    };

    class NFS4VehiclePhysics {
      public:
        NFS4VehiclePhysics(RaycastVehicle *vehicle, btRigidBody *chassis, NFS4PerformanceData const &perfData);

        void SetInput(float throttle, float brake, float steer, bool handbrake);
        bool SetGear(Gear gear);
        void Update(float deltaTime);

        [[nodiscard]] float GetRPM() const {
            return m_state.rpm;
        }
        [[nodiscard]] Gear GetGear() const {
            return m_state.gear;
        }
        [[nodiscard]] float GetSpeedKMH() const;

        // Debug accessors
        [[nodiscard]] NFS4VehicleState const &GetState() const {
            return m_state;
        }
        [[nodiscard]] NFS4PerformanceData const &GetPerformanceData() const {
            return m_perf;
        }
        [[nodiscard]] NFS4DebugData const &GetDebugData() const {
            return m_debugData;
        }
        [[nodiscard]] btRigidBody *GetChassis() const {
            return m_chassis;
        }
        [[nodiscard]] NFS4PhysicsToggles &GetToggles() {
            return m_toggles;
        }
        [[nodiscard]] NFS4PhysicsToggles const &GetToggles() const {
            return m_toggles;
        }

      private:
        RaycastVehicle *m_vehicle;
        btRigidBody *m_chassis;
        NFS4PerformanceData m_perf;
        NFS4VehicleState m_state;
        NFS4DebugData m_debugData;
        NFS4PhysicsToggles m_toggles;

        static constexpr float PHYSICS_TIMESTEP = 1.0f / 32.0f; // NFS4 runs at 32Hz

        // Road surface grip factors
        static constexpr std::array<float, 24> ROAD_FACTORS = {1.0f,  1.0f,  0.75f, 0.8f,  0.98f, 0.8f, 0.75f, 0.95f,
                                                               0.75f, 0.75f, 0.98f, 0.95f, 0.95f, 0.8f, 1.0f,  0.75f,
                                                               0.98f, 0.98f, 0.8f,  0.8f,  0.8f,  0.8f, 0.8f,  0.8f};

        // Core NFS4 physics functions
        [[nodiscard]] float TorqueForRPM(float rpm) const;
        [[nodiscard]] float GearEffectiveRatio(Gear gear) const;
        [[nodiscard]] float TractionPowertrain(float rpm) const;
        [[nodiscard]] float LongitudinalDragCoefficient() const;
        [[nodiscard]] float CalculateDrag() const;
        [[nodiscard]] float GearRPMToVelocity(Gear gear) const;
        [[nodiscard]] float RpmFromWheels() const;

        // Traction model pipeline
        void ProcessTractionModel();

        // Grip and tire model
        [[nodiscard]] float TireFactor() const;
        [[nodiscard]] float WheelSurfaceGripFactor(WheelData const &wheel) const;
        [[nodiscard]] float WheelBaseRoadGrip(WheelData const &wheel, float surfaceGrip) const;
        [[nodiscard]] float WheelBiasGrip(WheelData const &wheel, float grip) const;
        [[nodiscard]] float WheelDownforceFactor(WheelData const &wheel) const;
        [[nodiscard]] float WheelDownforceGrip(WheelData const &wheel, float baseGrip, float biasedGrip) const;
        [[nodiscard]] float ModelWheelGrip(WheelData const &wheel) const;
        [[nodiscard]] float WheelTraction(WheelData const &wheel) const;
        [[nodiscard]] WheelData CalculateWheelData(WheelPosition pos) const;

        // Steering
        [[nodiscard]] float SlipAngleFactor() const;
        [[nodiscard]] float CalculateSteeringAngle() const;
        [[nodiscard]] float AngularVelocityFactor() const;
        [[nodiscard]] btVector3 WheelPlanarVector(WheelData const &wheel) const;

        // Forces
        [[nodiscard]] btVector3 WheelForce(WheelData const &wheel) const;
        [[nodiscard]] float SteeringAcceleration(WheelData const &wheel, btVector3 const &planarVector) const;
        [[nodiscard]] btVector3 WheelLossOfGrip(WheelData const &wheel, btVector3 const &forces) const;
        [[nodiscard]] btVector3 HandbrakeLossOfGrip(WheelData const &wheel, btVector3 const &planarVector, float traction) const;
        [[nodiscard]] float BrakeForce(WheelData const &wheel) const;

        // Input processing
        void ProcessSteeringInput(float dt);
        void ProcessThrottleInput(float dt);
        void ProcessBrakeInput(float dt);
        void ProcessGearInput();

        // Utility
        [[nodiscard]] btVector3 GetLocalVelocity() const;
        [[nodiscard]] btVector3 GetLocalAngularVelocity() const;
        [[nodiscard]] float CalculateSlipAngle() const;
        [[nodiscard]] float CalculateSpeedXZ() const;
        [[nodiscard]] int IncrementHandbrakeAccumulator() const;
        [[nodiscard]] static float HandbrakeScalingFunction(float value);
        [[nodiscard]] float gTransferDamp() const;
        [[nodiscard]] btVector3 OrientationToGround() const;
        [[nodiscard]] float RoadFactor(WheelData const &wheel) const;
        [[nodiscard]] btVector3 AirborneDrag() const;
        [[nodiscard]] bool ShouldApplyNearStopDeceleration() const;
        [[nodiscard]] btVector3 DampLateralVelocity() const;
        [[nodiscard]] bool WentAirborne() const;
        struct TurningCircleResult {
            btVector3 angularVelocity;
            btVector3 linearVelocity;
        };
        [[nodiscard]] TurningCircleResult TurningCircle(btVector3 const &localAngularVelocity, btVector3 const &localVelocity) const;

        void ComputeBasisToRoad();
        void AdjustToRoad();
        void PreventSinking() const;
        void ApplyTurningCircle(float dt);
        void ApplyLateralVelocityDamping(float dt);
        void ApplyNeutralGearDeceleration(float dt);
        void ApplyNearStopDeceleration(float dt);
        void PreventMovingSideways();
        void ApplyAirborneDrag(float dt);
        void LimitAngularVelocity() const;
        void GoAirborne();
        void ApplyDownforce(float dt);

        [[nodiscard]] static bool IsFrontWheel(WheelPosition const pos) {
            return pos == WheelPosition::FRONT_LEFT || pos == WheelPosition::FRONT_RIGHT;
        }

        [[nodiscard]] static btVector3 RotateVectorY(btVector3 const &v, float const angle) {
            float const c = std::cos(angle * SIMD_2_PI);
            float const s = std::sin(angle * SIMD_2_PI);
            return {v.x() * c + v.z() * s, v.y(), -v.x() * s + v.z() * c};
        }
    };
} // namespace OpenNFS
