#include "NFS4VehiclePhysics.h"
#include "../Util/Utils.h"
#include "Car.h"
#include <algorithm>
#include <cmath>

namespace OpenNFS {
    NFS4PerformanceData NFS4PerformanceData::FromPhysicsData(LibOpenNFS::Car::PhysicsData const &physicsData) {
        NFS4PerformanceData perf{};

        // Copy scalar values
        perf.mass = physicsData.mass;
        perf.maxVelocity = physicsData.maximumVelocityOfCar;
        perf.engineMinRPM = static_cast<float>(physicsData.engineMinimumRpm);
        perf.engineRedlineRPM = static_cast<float>(physicsData.engineRedlineInRpm);
        perf.gasOffFactor = physicsData.gasOffFactor;
        perf.lateralGripMultiplier = physicsData.lateralAccelerationGripMultiplier;
        perf.frontGripBias = physicsData.frontGripBias;
        perf.frontDriveRatio = physicsData.frontDriveRatio;
        perf.frontBrakeBias = physicsData.frontBiasBrakeRatio;
        perf.understeerGradient = physicsData.understeerGradient;
        perf.minimumSteeringAcceleration = physicsData.minimumSteeringAcceleration;
        perf.turningCircleRadius = physicsData.turningCircleRadius;
        perf.downforceMult = physicsData.aerodynamicDownforceMultiplier;
        perf.gTransferFactor = physicsData.gTransferFactor;
        perf.turnInRamp = physicsData.turnInRamp;
        perf.turnOutRamp = physicsData.turnOutRamp;
        perf.maxBrakingDeceleration = physicsData.maximumBrakingDeceleration;
        perf.gearShiftDelay = physicsData.gearShiftDelay;
        perf.maxGear = physicsData.numberOfGearsManual - 1; // -1 because gear array is 0-indexed
        perf.hasSpoiler = physicsData.spoilerFunctionType > 0;
        perf.hasABS = physicsData.usesAntilockBrakeSystem;

        // Torque curve (21 values)
        for (size_t i = 0; i < std::min(physicsData.torqueCurve.size(), perf.torqueCurve.size()); ++i) {
            perf.torqueCurve[i] = physicsData.torqueCurve[i];
        }

        // Gear velocity to RPM ratio (8 values)
        for (size_t i = 0; i < std::min(physicsData.velocityToRpmRatioManual.size(), perf.gearVelocityToRPM.size()); ++i) {
            perf.gearVelocityToRPM[i] = physicsData.velocityToRpmRatioManual[i];
        }

        // Gear efficiency (8 values)
        for (size_t i = 0; i < std::min(physicsData.gearEfficiencyManual.size(), perf.gearEfficiency.size()); ++i) {
            perf.gearEfficiency[i] = physicsData.gearEfficiencyManual[i];
        }

        // Shift blip RPM (8 values)
        for (size_t i = 0; i < std::min(physicsData.shiftBlipInRpm.size(), perf.shiftBlipRPM.size()); ++i) {
            perf.shiftBlipRPM[i] = static_cast<float>(physicsData.shiftBlipInRpm[i]);
        }

        // Brake blip RPM (8 values)
        for (size_t i = 0; i < std::min(physicsData.brakeBlipInRpm.size(), perf.brakeBlipRPM.size()); ++i) {
            perf.brakeBlipRPM[i] = static_cast<float>(physicsData.brakeBlipInRpm[i]);
        }

        // Brake increasing curve (8 values) - convert from float to uint8_t
        for (size_t i = 0; i < std::min(physicsData.brakeIncreasingCurve.size(), perf.brakeIncreasingCurve.size()); ++i) {
            perf.brakeIncreasingCurve[i] = static_cast<uint8_t>(physicsData.brakeIncreasingCurve[i]);
        }

        // Brake decreasing curve (8 values) - convert from float to uint8_t
        for (size_t i = 0; i < std::min(physicsData.brakeDecreasingCurve.size(), perf.brakeDecreasingCurve.size()); ++i) {
            perf.brakeDecreasingCurve[i] = static_cast<uint8_t>(physicsData.brakeDecreasingCurve[i]);
        }

        return perf;
    }

    NFS4VehiclePhysics::NFS4VehiclePhysics(RaycastVehicle *vehicle, btRigidBody *chassis, NFS4PerformanceData const &perfData)
        : m_vehicle(vehicle), m_chassis(chassis), m_perf(perfData), m_state({}), m_debugData({}) {
        m_state.rpm = perfData.engineMinRPM;
        m_state.gear = Gear::GEAR_1;
        m_state.weather = Weather::DRY;
        m_state.hasContactWithGround = false;
    }

    void NFS4VehiclePhysics::SetInput(float const throttle, float const brake, float const steer, bool const handbrake) {
        m_state.throttleInput = std::clamp(throttle, 0.0f, 1.0f);
        m_state.brakeInput = std::clamp(brake, 0.0f, 1.0f);
        m_state.steerInput = std::clamp(steer, -1.0f, 1.0f);
        m_state.handbrakeInput = handbrake;
    }

    bool NFS4VehiclePhysics::SetGear(Gear const gear) {
        if ((int)gear <= m_perf.maxGear) {
            m_state.requestedGear = gear;
            return true;
        }
        return false;
    }

    float NFS4VehiclePhysics::GetSpeedKMH() const {
        return m_chassis->getLinearVelocity().length() * 3.6f;
    }

    // Core Powertrain
    float NFS4VehiclePhysics::TorqueForRPM(float const rpm) const {
        float const torqueDiv = rpm / 500.0f;
        int const torqueIdx = std::clamp(static_cast<int>(std::floor(torqueDiv)), 0, 19);
        int const torqueNext = torqueIdx + 1;
        float const factor = torqueDiv - torqueIdx;
        return std::lerp(m_perf.torqueCurve[torqueIdx], m_perf.torqueCurve[torqueNext], factor);
    }

    float NFS4VehiclePhysics::GearEffectiveRatio(Gear gear) const {
        int const idx = static_cast<int>(gear);
        float const velocityToRPM = m_perf.gearVelocityToRPM[idx];
        float const gearEfficiency = m_perf.gearEfficiency[idx];
        return velocityToRPM * gearEfficiency / (10.0f * m_perf.mass);
    }

    float NFS4VehiclePhysics::TractionPowertrain(float const rpm) const {
        float const torque = TorqueForRPM(rpm);
        float const gearRatio = GearEffectiveRatio(m_state.gear);
        float torqueOutput = torque * gearRatio;

        // NFS4 clamps torque output per gear
        if (m_state.gear == Gear::GEAR_1) {
            torqueOutput = std::min(torqueOutput, 10.0f);
        } else if (m_state.gear == Gear::REVERSE) {
            torqueOutput = std::max(torqueOutput, -6.0f);
        } else {
            torqueOutput = std::min(torqueOutput, 8.0f);
        }
        return torqueOutput;
    }

    float NFS4VehiclePhysics::LongitudinalDragCoefficient() const {
        constexpr float COEFF1 = 3.0f / 2.0f;
        constexpr float COEFF2 = 0.98f;
        constexpr float COEFF3 = 0.36757159f;

        float const velocityToRPM = m_perf.gearVelocityToRPM[m_perf.maxGear];
        float const rpmForMaxSpeed = velocityToRPM * m_perf.maxVelocity;
        int const rpmIdx = static_cast<int>(std::floor(rpmForMaxSpeed / 500.0f));
        float const torqueForMaxSpeed = m_perf.torqueCurve[rpmIdx];

        float result = torqueForMaxSpeed * velocityToRPM / (10.0f * m_perf.mass);
        result = COEFF1 * result * COEFF2 / (std::pow(m_perf.maxVelocity, 3) * COEFF3);
        return result;
    }

    float NFS4VehiclePhysics::CalculateDrag() const {
        constexpr float DRAG_INIT = 0.2450477f;
        btVector3 const velLocal = GetLocalVelocity();

        float result = DRAG_INIT * LongitudinalDragCoefficient();
        float const velocityMaxDiff = std::abs(velLocal.z()) - m_perf.maxVelocity;
        result = result * std::pow(velLocal.z(), 3);

        if (velocityMaxDiff > 0) {
            result += std::pow(velocityMaxDiff, 2) * 0.01f;
        }
        return result;
    }

    float NFS4VehiclePhysics::GearRPMToVelocity(Gear const gear) const {
        return 1.0f / m_perf.gearVelocityToRPM[static_cast<int>(gear)];
    }

    float NFS4VehiclePhysics::RpmFromWheels() const {
        btVector3 const velLocal = GetLocalVelocity();
        int const idx = static_cast<int>(m_state.gear);
        return velLocal.z() * m_perf.gearVelocityToRPM[idx];
    }

    // Slip Angle and Steering
    float NFS4VehiclePhysics::CalculateSlipAngle() const {
        constexpr float VELOCITY_THRESHOLD = 0.5f;
        btVector3 const velLocal = GetLocalVelocity();

        if (std::abs(velLocal.z()) > VELOCITY_THRESHOLD) {
            return velLocal.x() / velLocal.z();
        }
        return 0.0f;
    }

    float NFS4VehiclePhysics::CalculateSpeedXZ() const {
        btVector3 const velLocal = GetLocalVelocity();
        btVector3 const velXZ(velLocal.x(), 0, velLocal.z());
        float const speed = velXZ.length();
        return velLocal.z() > 0 ? speed : -speed;
    }

    float NFS4VehiclePhysics::SlipAngleFactor() const {
        constexpr float MEDIUM_VELOCITY_THRESHOLD = 13.4f;
        constexpr float HIGH_VELOCITY_THRESHOLD = 26.7f;
        constexpr float MEDIUM_SLIP_THRESHOLD = 0.15f;
        constexpr float HIGH_SLIP_THRESHOLD = 0.05f;

        float const steering = m_state.currentSteering;
        btVector3 const velLocal = GetLocalVelocity();
        float const slipAngle = m_state.slipAngle;
        bool const isSameDir = (slipAngle * steering) >= 0;

        float result = 1.0f;

        if (!m_state.handbrakeInput && isSameDir) {
            if (velLocal.z() > HIGH_VELOCITY_THRESHOLD) {
                if (std::abs(slipAngle) > HIGH_SLIP_THRESHOLD) {
                    result = 2.0f * std::abs(slipAngle);
                }
            } else if (velLocal.z() > MEDIUM_VELOCITY_THRESHOLD) {
                if (std::abs(slipAngle) > MEDIUM_SLIP_THRESHOLD) {
                    result = 2.0f * std::abs(slipAngle);
                }
            }
            result = std::min(1.0f, result);
        } else if (m_state.handbrakeInput && !isSameDir) {
            result = 0.85f - std::abs(velLocal.z()) * 0.0056f;
            result = std::max(result, 0.55f);
        }

        return result;
    }

    float NFS4VehiclePhysics::CalculateSteeringAngle() const {
        if (!m_state.hasContactWithGround) {
            return 0.0f;
        }

        float steering = m_state.currentSteering;
        btVector3 const velLocal = GetLocalVelocity();
        float const slipFactor = SlipAngleFactor();
        float const steeringAccel = m_perf.minimumSteeringAcceleration;

        steering *= slipFactor;

        if (std::abs(velLocal.z()) > 40.0f) {
            float velocityFactor = 0.015f * std::abs(velLocal.z());
            velocityFactor = std::clamp(velocityFactor, 1.0f, 1.5f);
            steering = steering / velocityFactor;
        }

        float const result = steeringAccel * 1.5f * 0.00277777777f * steering * 0.0078125f;
        return std::clamp(result, -1.0f, 1.0f);
    }

    float NFS4VehiclePhysics::AngularVelocityFactor() const {
        constexpr float FACTOR_A = 0.0125f;
        constexpr float OFFSET_A = -0.25f;
        constexpr float UPPER_LIMIT = 1.0f;
        constexpr float LOWER_LIMIT = 0.1f;

        if (!m_state.hasContactWithGround) {
            return 0.0f;
        }

        btVector3 const angVel = GetLocalAngularVelocity();
        btVector3 const inertiaInv = m_chassis->getInvInertiaDiagLocal();
        float const mass = m_perf.mass;
        float const speedXZ = std::abs(m_state.speedXZ);

        float velocityFactor = speedXZ * FACTOR_A + OFFSET_A;
        velocityFactor = std::clamp(velocityFactor, LOWER_LIMIT, UPPER_LIMIT);

        float result = -velocityFactor * std::abs(angVel.y()) * 32.0f * mass * inertiaInv.y();

        if (m_state.gear == Gear::REVERSE) {
            result /= 2.0f;
        }

        return -result;
    }

    // Tire/Grip Model
    float NFS4VehiclePhysics::TireFactor() const {
        constexpr float tireTuning = 1.0f; // TODO: expose as tuning parameter
        float result;

        switch (m_state.weather) {
        case Weather::DRY:
            result = 28.0f - (tireTuning - 0.8f) * 40.0f;
            result = 1.0f / std::max(16.0f, result);
            break;
        case Weather::RAIN:
            result = 16.0f - (tireTuning - 0.8f) * 32.0f;
            result = 3.0f / std::max(6.0f, result);
            break;
        case Weather::SNOW:
            result = 12.0f - (tireTuning - 0.8f) * 24.0f;
            result = 3.0f / std::max(4.0f, result);
            break;
        }
        return result;
    }

    float NFS4VehiclePhysics::WheelSurfaceGripFactor(WheelData const &wheel) const {
        btVector3 const ortToGround = OrientationToGround();
        float const slope = std::clamp(ortToGround.y(), 0.75f, 1.0f);
        float const roadFactor = (ROAD_FACTORS[wheel.roadSurface] + 1.0f) * 0.5f;

        float weatherFactor = 1.0f;
        if (m_state.weather == Weather::RAIN)
            weatherFactor = 0.9f;
        else if (m_state.weather == Weather::SNOW)
            weatherFactor = 0.8f;

        float const effectiveWeatherFactor = slope * roadFactor * weatherFactor * 0.25f;
        return (effectiveWeatherFactor + slope) * roadFactor;
    }

    float NFS4VehiclePhysics::WheelBaseRoadGrip(WheelData const &wheel, float const surfaceGrip) const {
        // Get gravity in local space
        btVector3 const gravity(0, -9.81f, 0);
        btTransform trans = m_chassis->getWorldTransform();
        btVector3 const localGravity = trans.getBasis().inverse() * gravity;

        return -localGravity.y() * m_perf.lateralGripMultiplier * surfaceGrip;
    }

    float NFS4VehiclePhysics::WheelBiasGrip(WheelData const &wheel, float const grip) const {
        float const frontBias = m_perf.frontGripBias;
        float const rearBias = 1.0f - frontBias;

        if (IsFrontWheel(wheel.type)) {
            return grip * frontBias;
        }
        return grip * rearBias;
    }

    float NFS4VehiclePhysics::WheelDownforceFactor(WheelData const &wheel) const {
        constexpr float DOWNFORCE_THRESHOLD_SPEED = 10.0f;
        btVector3 const velLocal = GetLocalVelocity();
        float downforce = m_perf.downforceMult;

        if (std::abs(velLocal.z()) <= DOWNFORCE_THRESHOLD_SPEED) {
            return downforce + 1.0f;
        }

        downforce = velLocal.z() * downforce;
        float rearFactor = 1.5f;

        if (m_state.brake >= 0.15f && m_perf.hasSpoiler) {
            rearFactor = 1.75f;
        }

        if (IsFrontWheel(wheel.type)) {
            downforce += 1.0f;
        } else {
            downforce = downforce * rearFactor + 1.0f;
        }

        return std::max(downforce, 0.0f);
    }

    float NFS4VehiclePhysics::WheelDownforceGrip(WheelData const &wheel, float const baseGrip, float const biasedGrip) const {
        float const downforce = WheelDownforceFactor(wheel);
        float const gTransfer = gTransferDamp();

        if (IsFrontWheel(wheel.type)) {
            return (biasedGrip - gTransfer) * downforce;
        }
        return (baseGrip - biasedGrip + gTransfer) * downforce;
    }

    float NFS4VehiclePhysics::ModelWheelGrip(WheelData const &wheel) const {
        float const surfaceGrip = WheelSurfaceGripFactor(wheel);
        float const baseGrip = WheelBaseRoadGrip(wheel, surfaceGrip);
        float const biasedGrip = WheelBiasGrip(wheel, baseGrip);
        return WheelDownforceGrip(wheel, baseGrip, biasedGrip);
    }

    float NFS4VehiclePhysics::gTransferDamp() const {
        float factor = m_state.gTransfer;

        if (factor < 0.0f) {
            factor *= 0.75f;
            if (m_state.weather != Weather::DRY) {
                factor *= 1.1f;
            }
        } else {
            if (m_state.weather != Weather::DRY) {
                factor *= 0.9f;
            }
        }
        return factor;
    }

    // Wheel Forces
    btVector3 NFS4VehiclePhysics::WheelPlanarVector(WheelData const &wheel) const {
        btVector3 const velLocal = GetLocalVelocity();
        btVector3 planarVector = -0.5f * velLocal * 32.0f;
        float const angVelFactor = AngularVelocityFactor();

        if (IsFrontWheel(wheel.type)) {
            planarVector.setX(planarVector.x() + angVelFactor);
        } else {
            planarVector.setX(planarVector.x() - angVelFactor);
        }
        planarVector.setY(0);
        return planarVector;
    }

    float NFS4VehiclePhysics::SteeringAcceleration(WheelData const &wheel, btVector3 const &planarVector) const {
        float const angle = std::atan2(planarVector.x(), std::abs(planarVector.z()));
        float const gripLoss = wheel.grip - wheel.grip * TireFactor();

        float value;
        if (m_state.gear == Gear::REVERSE) {
            value = std::sin(angle / 2.0f) * gripLoss * 2.0f;
        } else {
            value = std::sin(angle / 2.0f) * gripLoss * 4.0f;
        }
        return value;
    }

    btVector3 NFS4VehiclePhysics::WheelLossOfGrip(WheelData const &wheel, btVector3 const &forces) const {
        float const forceMagnitude = forces.length();
        float const lateralGrip = wheel.grip;
        float const tFactor = TireFactor();
        float const gripLoss = lateralGrip - lateralGrip * tFactor;

        btVector3 result = forces;

        if (gripLoss < forceMagnitude) {
            float const diff = forceMagnitude - gripLoss;
            float const val = std::min(diff, gripLoss) * tFactor;
            float factor = (gripLoss - val) / forceMagnitude;

            if (forceMagnitude <= (gripLoss - val)) {
                factor = 1.0f;
            }

            bool const isFront = IsFrontWheel(wheel.type);

            // Wheelspin in first gear at high throttle (only on high-grip surfaces)
            if (!isFront && m_state.gear == Gear::GEAR_1 && m_state.throttle > 0.85f &&
                m_state.rpm >= m_perf.engineRedlineRPM * 0.85f && RoadFactor(wheel) >= 0.95f) {
                result.setZ(0.5f * factor * forces.z());
                result.setX(factor * forces.x());
            }
        }
        return result;
    }

    int NFS4VehiclePhysics::IncrementHandbrakeAccumulator() const {
        int increment{0};
        switch (m_state.weather) {
        case Weather::DRY:
            increment = 1;
            break;
        case Weather::RAIN:
            increment = 3;
            break;
        case Weather::SNOW:
            increment = 4;
            break;
        }
        return std::min(m_state.handbrakeAccumulator + increment, 384);
    }

    float NFS4VehiclePhysics::HandbrakeScalingFunction(float const value) {
        // Exact x87 FPU sequence from NFS4
        constexpr float fldl2e = 1.442695040888963f;
        float const x = fldl2e * (value - 0.5f) * -12.56636f;
        float const fprem = std::fmod(x, 1.0f);
        float const f2xm1 = std::pow(2.0f, fprem) - 1.0f;
        float const fscale = (f2xm1 + 1.0f) * std::pow(2.0f, std::floor(std::abs(x)) * (x >= 0 ? 1.0f : -1.0f));
        return 1.0f - 0.8f / (1.0f + fscale);
    }

    btVector3 NFS4VehiclePhysics::HandbrakeLossOfGrip(WheelData const &wheel, btVector3 const &planarVector, float const traction) const {
        float const grip = wheel.grip;
        float const downforce = wheel.downforce;
        float const xzSpeed = btVector3(planarVector.x(), 0, planarVector.z()).length();

        float const gripTimesDownforce = grip * downforce;
        float const gripLoss = gripTimesDownforce - gripTimesDownforce * TireFactor();

        float factor = 0.75f;
        if (gripLoss < xzSpeed) {
            factor = std::abs(gripLoss / xzSpeed);
        }

        float handbrakeFactor = 1.0f;
        if (m_state.handbrakeAccumulator > 0) {
            handbrakeFactor = HandbrakeScalingFunction(m_state.handbrakeAccumulator / 384.0f) * 0.75f;
        }

        float lateralForce = handbrakeFactor * planarVector.x() * factor;
        float longitudinalForce = traction * factor;

        if (m_state.gear == Gear::NEUTRAL) {
            lateralForce *= 0.05f;
            longitudinalForce *= 0.05f;
        }

        return {lateralForce, 0, longitudinalForce};
    }

    btVector3 NFS4VehiclePhysics::WheelForce(WheelData const &wheel) const {
        btVector3 const velLocal = GetLocalVelocity();
        float traction = wheel.traction;
        bool const handbrake = m_state.handbrakeInput || m_state.lostGrip;
        float const speedXZ = m_state.speedXZ;

        btVector3 planarVec = WheelPlanarVector(wheel);
        bool const isFront = IsFrontWheel(wheel.type);
        btVector3 forces;

        float const steering = isFront ? m_state.steeringAngle : 0.0f;
        planarVec = RotateVectorY(planarVec, -steering);

        // Longitudinal acceleration calculation
        bool tractionLoss = false;
        if (traction >= 0.0f || planarVec.z() >= 0.0f) {
            if (traction > 0.0f && planarVec.z() > 0.0f &&
                (m_state.throttle < 0.25f || m_state.gear == Utils::OneOf<Gear::REVERSE, Gear::NEUTRAL>())) {
                traction = std::min(traction, planarVec.z());
                tractionLoss = true;
            }
        } else if (m_state.throttle < 0.25f || m_state.gear != Gear::REVERSE) {
            traction = std::max(traction, planarVec.z());
            tractionLoss = true;
        }

        traction *= m_perf.lateralGripMultiplier;

        // Main force calculation
        if ((!handbrake || isFront) && (!handbrake || !isFront || !tractionLoss || std::abs(traction) <= wheel.grip) ||
            speedXZ < 2.2351501f) {

            // ABS
            if (tractionLoss && wheel.grip < std::abs(traction) && !handbrake && m_perf.hasABS) {
                traction = std::clamp(traction, -wheel.grip, wheel.grip);
            }

            float const value = SteeringAcceleration(wheel, planarVec);
            float steeringAccel = m_perf.minimumSteeringAcceleration * 2.0f;
            steeringAccel = std::min(std::abs(value), steeringAccel);

            // Low speed, low throttle reduces steering
            if (std::abs(velLocal.z()) < 13.4f &&
                ((m_state.throttle < 0.02f && std::abs(m_state.currentSteering) >= 127) || m_state.throttle < 0.015f)) {
                steeringAccel *= 0.25f;
            }

            float const understeerGrad = m_perf.understeerGradient;
            float understeer = (planarVec.x() >= 0 ? 1.0f : -1.0f) * std::min(steeringAccel, std::abs(planarVec.x()));

            if (isFront) {
                if (handbrake && std::abs(velLocal.z()) < 0.5f) {
                    understeer = 0.0f;
                }
                understeer = understeer * 0.8f * understeerGrad;
            } else {
                understeer = (understeer * 1.4f) / understeerGrad;
            }

            btVector3 f(understeer, 0, traction);
            f = WheelLossOfGrip(wheel, f);
            forces = RotateVectorY(f, -steering);
        } else {
            forces = HandbrakeLossOfGrip(wheel, planarVec, traction);
        }

        return forces;
    }

    float NFS4VehiclePhysics::BrakeForce(WheelData const &wheel) const {
        float brakeDecel = m_state.brake * m_perf.maxBrakingDeceleration;
        btVector3 const velLocal = GetLocalVelocity();
        float const brakeMax = std::abs(32.0f * velLocal.z());

        brakeDecel = std::min(brakeMax, brakeDecel);

        if (m_state.brake > 0.15f && m_perf.hasSpoiler) {
            brakeDecel += std::abs(velLocal.z()) * m_perf.downforceMult;
        }

        float frontBrakeRatio = m_perf.frontBrakeBias;
        if (m_state.handbrakeInput || m_state.lostGrip) {
            frontBrakeRatio += 0.05f;
        }

        if (IsFrontWheel(wheel.type)) {
            brakeDecel *= frontBrakeRatio;
        } else {
            brakeDecel *= (1.0f - frontBrakeRatio);
        }

        return (velLocal.z() >= 0 ? 1.0f : -1.0f) * brakeDecel;
    }

    float NFS4VehiclePhysics::WheelTraction(WheelData const &wheel) const {
        float traction = m_state.tractionForce;
        float const frontDrive = m_perf.frontDriveRatio;

        if (IsFrontWheel(wheel.type)) {
            traction *= frontDrive;
        } else {
            traction *= (1.0f - frontDrive);
        }

        // Create temp wheel data for brake force calc
        WheelData const tempWheel = wheel;
        return traction - BrakeForce(tempWheel);
    }

    WheelData NFS4VehiclePhysics::CalculateWheelData(WheelPosition const pos) const {
        WheelData data{};
        data.type = pos;
        data.roadSurface = 0; // TODO: Get from raycast
        data.grip = ModelWheelGrip(data);
        data.traction = WheelTraction(data);
        data.downforce = WheelDownforceFactor(data);
        return data;
    }

    // Input Processing
    void NFS4VehiclePhysics::ProcessSteeringInput(float const dt) {
        float steering = m_state.currentSteering;
        float const turnInRamp = m_perf.turnInRamp;
        float const turnOutRamp = m_perf.turnOutRamp;
        float const steeringTarget = 128.0f * m_state.steerInput;
        float const steeringDiff = steeringTarget - steering;

        float delta;
        if ((steeringDiff < 0 && steering > 0) || (steeringDiff > 0 && steering < 0)) {
            delta = turnOutRamp;
        } else {
            delta = turnInRamp;
        }

        // Move toward target
        float const maxDelta = delta * 32.0f * dt;
        if (std::abs(steeringDiff) <= maxDelta) {
            steering = steeringTarget;
        } else {
            steering += (steeringDiff > 0 ? 1.0f : -1.0f) * maxDelta;
        }

        m_state.currentSteering = steering;
    }

    void NFS4VehiclePhysics::ProcessThrottleInput(float const dt) {
        constexpr float delta = 16.0f / 255.0f;
        float const maxDelta = delta * 32.0f * dt;
        float const diff = m_state.throttleInput - m_state.throttle;

        if (std::abs(diff) <= maxDelta) {
            m_state.throttle = m_state.throttleInput;
        } else {
            m_state.throttle += (diff > 0 ? 1.0f : -1.0f) * maxDelta;
        }
    }

    void NFS4VehiclePhysics::ProcessBrakeInput(float const dt) {
        int const idx = static_cast<int>(m_state.brake * 255) >> 5;
        float delta;

        if (m_state.brakeInput < m_state.brake) {
            delta = m_perf.brakeDecreasingCurve[idx] / 255.0f;
        } else {
            delta = m_perf.brakeIncreasingCurve[idx] / 255.0f;
        }

        float const maxDelta = delta * 32.0f * dt;
        float const diff = m_state.brakeInput - m_state.brake;

        if (std::abs(diff) <= maxDelta) {
            m_state.brake = m_state.brakeInput;
        } else {
            m_state.brake += (diff > 0 ? 1.0f : -1.0f) * maxDelta;
        }
    }

    void NFS4VehiclePhysics::ProcessGearInput() {
        if (m_state.requestedGear != m_state.gear) {
            m_state.gearShiftCounter = m_perf.gearShiftDelay;
            m_state.shiftedDown = m_state.requestedGear < m_state.gear && m_state.requestedGear > Gear::NEUTRAL;
            m_state.gear = m_state.requestedGear;
        }

        if (m_state.gearShiftCounter > 0) {
            m_state.gearShiftCounter--;
        }
    }

    // Utility Functions
    btVector3 NFS4VehiclePhysics::GetLocalVelocity() const {
        btVector3 const worldVel = m_chassis->getLinearVelocity();
        btTransform trans = m_chassis->getWorldTransform();
        return trans.getBasis().inverse() * worldVel;
    }

    btVector3 NFS4VehiclePhysics::GetLocalAngularVelocity() const {
        btVector3 const worldAngVel = m_chassis->getAngularVelocity();
        btTransform trans = m_chassis->getWorldTransform();
        return trans.getBasis().inverse() * worldAngVel;
    }

    btVector3 NFS4VehiclePhysics::OrientationToGround() const {
        btTransform trans = m_chassis->getWorldTransform();
        btVector3 const up(0, 1, 0); // World up (TODO: Get from road normal?)
        btMatrix3x3 const basis = trans.getBasis();
        return {basis.getColumn(0).dot(up), basis.getColumn(1).dot(up), basis.getColumn(2).dot(up)};
    }

    float NFS4VehiclePhysics::RoadFactor(WheelData const &wheel) const {
        // Weather factors: DRY = 1.0, RAIN = 0.87, SNOW = 0.75
        constexpr std::array<float, 3> WEATHER_FACTORS = {1.0f, 0.87f, 0.75f};
        return ROAD_FACTORS[wheel.roadSurface] * WEATHER_FACTORS[static_cast<int>(m_state.weather)];
    }

    // Main Update Loop
    void NFS4VehiclePhysics::Update(float const deltaTime) {
        // Update state from Bullet
        m_state.slipAngle = CalculateSlipAngle();
        m_state.speedXZ = CalculateSpeedXZ();

        // Manually trigger raycast update to get fresh contact data
        m_vehicle->updateWheelTransformsAndContacts();

        // Check ground contact using RaycastVehicle wheel info
        m_state.hasContactWithGround = false;
        for (int i = 0; i < m_vehicle->getNumWheels(); i++) {
            btWheelInfo const &wheel = m_vehicle->getWheelInfo(i);
            m_debugData.wheelInContact[i] = wheel.m_raycastInfo.m_isInContact;
            m_debugData.wheelSuspensionLength[i] = wheel.m_raycastInfo.m_suspensionLength;
            if (wheel.m_raycastInfo.m_isInContact) {
                m_state.hasContactWithGround = true;
            }
        }

        // Process inputs
        ProcessSteeringInput(deltaTime);
        ProcessThrottleInput(deltaTime);
        ProcessBrakeInput(deltaTime);
        ProcessGearInput();

        // Calculate steering angle
        m_state.steeringAngle = CalculateSteeringAngle();

        // Run traction model to get engine force
        ProcessTractionModel();

        // Calculate wheel forces
        std::array<WheelData, 4> wheels{};
        std::array<btVector3, 4> wheelForces{};

        wheels[0] = CalculateWheelData(WheelPosition::FRONT_LEFT);
        wheels[1] = CalculateWheelData(WheelPosition::FRONT_RIGHT);
        wheels[2] = CalculateWheelData(WheelPosition::REAR_LEFT);
        wheels[3] = CalculateWheelData(WheelPosition::REAR_RIGHT);

        for (int i = 0; i < 4; i++) {
            wheelForces[i] = WheelForce(wheels[i]);
        }

        // Sum forces and convert to world space
        btVector3 totalForce(0, 0, 0);
        btVector3 totalTorque(0, 0, 0);

        btTransform trans = m_chassis->getWorldTransform();
        btVector3 const ortToGround = OrientationToGround();
        float const slope = ortToGround.y();

        for (int i = 0; i < 4; i++) {
            btVector3 slopedForce = wheelForces[i] * slope;
            totalForce += slopedForce * 0.5f;
        }

        // Angular acceleration from wheel forces (simplified)
        float const angAccelY = ((wheelForces[0].x() + wheelForces[1].x()) - (wheelForces[2].x() + wheelForces[3].x())) * 0.5f * 4.0f *
                                m_perf.mass * m_chassis->getInvInertiaDiagLocal().y() * deltaTime;

        totalTorque.setY(angAccelY * slope);

        // Adjust longitudinal force
        totalForce.setZ(totalForce.z() / m_perf.lateralGripMultiplier);

        // Convert to world space and apply
        btVector3 const worldForce = trans.getBasis() * totalForce;
        btVector3 const worldTorque = trans.getBasis() * totalForce;

        // Apply forces to chassis
        m_chassis->applyCentralForce(worldForce * m_perf.mass);
        m_chassis->applyTorque(worldTorque);

        // Update g-transfer for next frame
        m_state.gTransfer = totalForce.z() * m_perf.gTransferFactor;

        // Handbrake accumulator
        if (m_state.handbrakeInput || m_state.lostGrip) {
            m_state.handbrakeAccumulator = IncrementHandbrakeAccumulator();
        } else {
            m_state.handbrakeAccumulator = 0;
        }

        // Apply steering to btRaycastVehicle for visual wheel rotation
        m_vehicle->setSteeringValue(m_state.steeringAngle * SIMD_2_PI, FRONT_LEFT);
        m_vehicle->setSteeringValue(m_state.steeringAngle * SIMD_2_PI, FRONT_RIGHT);

        // Update debug data
        m_debugData.wheels = wheels;
        m_debugData.wheelForces = wheelForces;
        m_debugData.totalForce = totalForce;
        m_debugData.totalTorque = totalTorque;
        m_debugData.localVelocity = GetLocalVelocity();
        m_debugData.localAngularVelocity = GetLocalAngularVelocity();
        m_debugData.orientationToGround = ortToGround;
        m_debugData.drag = CalculateDrag();
        m_debugData.torque = TorqueForRPM(m_state.rpm);
        m_debugData.slipAngleFactor = SlipAngleFactor();
        m_debugData.tireFactor = TireFactor();
        m_debugData.angularVelocityFactor = AngularVelocityFactor();

        // Store wheel world positions for debug visualisation
        for (int i = 0; i < 4; i++) {
            btWheelInfo const &wheelInfo = m_vehicle->getWheelInfo(i);
            m_debugData.wheelWorldPositions[i] = wheelInfo.m_worldTransform.getOrigin();
        }

        // Update history buffers for graphs
        size_t const idx = m_debugData.historyIndex;
        m_debugData.rpmHistory[idx] = m_state.rpm;
        m_debugData.speedHistory[idx] = GetSpeedKMH();
        m_debugData.throttleHistory[idx] = m_state.throttle;
        m_debugData.brakeHistory[idx] = m_state.brake;
        m_debugData.steeringHistory[idx] = m_state.currentSteering / 128.0f; // Normalise to -1..1
        m_debugData.slipAngleHistory[idx] = m_state.slipAngle;
        m_debugData.tractionForceHistory[idx] = m_state.tractionForce;
        m_debugData.gTransferHistory[idx] = m_state.gTransfer;
        m_debugData.historyIndex = (idx + 1) % NFS4DebugData::HISTORY_SIZE;
    }

    void NFS4VehiclePhysics::ProcessTractionModel() {
        float rpm = m_state.rpm;
        float force = 0.0f;
        bool lostGrip = m_state.handbrakeInput;
        int handbrakeAccumulator = m_state.handbrakeAccumulator;
        float const drag = CalculateDrag();
        btVector3 const velLocal = GetLocalVelocity();

        // Step 1: RPM above redline check
        bool const aboveRedline = rpm > m_perf.engineRedlineRPM;
        if ((4.0f * m_perf.engineRedlineRPM / 3.0f) < rpm) {
            lostGrip = true;
        }
        rpm = std::min(rpm, m_perf.engineRedlineRPM);

        // Step 2: Calculate target RPM from throttle
        float targetRPM = m_perf.engineRedlineRPM * m_state.throttle;
        targetRPM = std::clamp(targetRPM, m_perf.engineMinRPM, m_perf.engineRedlineRPM);

        bool const inGear = m_state.gear != Gear::NEUTRAL && m_state.hasContactWithGround && m_state.gearShiftCounter == 0;

        if (inGear) {
            // Step 3a: In gear and grounded

            // Limit gear when low velocity
            float const rpmToVel = GearRPMToVelocity(Gear::GEAR_1);
            float const maxVelAtLowestGear = m_perf.engineRedlineRPM * rpmToVel;
            if (velLocal.z() < maxVelAtLowestGear && m_state.gear > Gear::GEAR_3) {
                m_state.gear = Gear::GEAR_3;
            }

            // Traction model force calculation
            rpm = std::min(m_perf.engineRedlineRPM, rpm);
            force = TractionPowertrain(rpm);

            float const rpmFromWheelsVal = std::round(std::abs(RpmFromWheels()));
            float rpmTargetWheelsDiff = targetRPM - rpmFromWheelsVal;

            // Deadband around target RPM
            if (std::abs(rpmTargetWheelsDiff) < 500 && targetRPM < (m_perf.engineRedlineRPM - 300)) {
                rpmTargetWheelsDiff = 0;
            }

            float const rpmDiff = rpm - rpmFromWheelsVal;

            // Check for over-revving or going in wrong direction
            bool const rpmDiffTooBig = (m_perf.engineRedlineRPM / 6.0f) < rpmDiff &&
                                        m_state.gearShiftCounter == 0 && m_state.gear < Gear::GEAR_4;
            bool const goingInReverseDir =
                (m_state.gear > Gear::NEUTRAL && m_state.speedXZ < -0.3f && m_state.throttle > (8.0f / 255.0f)) ||
                (m_state.gear == Gear::REVERSE && m_state.speedXZ > 0.3f && m_state.throttle > (8.0f / 255.0f));

            if (rpmDiffTooBig || goingInReverseDir) {
                // Engine over-revving or going wrong direction - reduce RPM
                constexpr std::array<int, 8> adjustByGear = {50, 50, 10, 15, 20, 22, 50, 50};
                int rpmAdjust = 125;
                if (targetRPM >= 2000) {
                    rpmAdjust = adjustByGear[static_cast<int>(m_state.gear)];
                }
                rpm -= std::min(static_cast<float>(rpmAdjust), rpmDiff);

            } else if (rpmTargetWheelsDiff < 0) {
                // Coasting / engine braking (wheels spinning faster than engine wants)
                force = -force * m_perf.gasOffFactor;

                if (m_state.gear < Gear::NEUTRAL || velLocal.z() > 0) {
                    if (m_state.gear == Gear::REVERSE && velLocal.z() > 0) {
                        force = -std::abs(force);
                    }
                } else {
                    force = std::abs(force);
                }

                // RPM adjustment during coasting
                float const rpmAdjust = std::floor(GearRPMToVelocity(m_state.gear) * 8.0f * 10240.0f);
                rpm += std::min(rpmAdjust, -rpmDiff);

                if (!aboveRedline) {
                    rpm = std::max({targetRPM, rpm, m_perf.engineMinRPM});
                } else {
                    rpm = m_perf.engineRedlineRPM;
                }

                // Check velocity against redline for this gear
                float velocityRedline = m_perf.engineRedlineRPM * GearRPMToVelocity(m_state.gear) * 1.15f;

                if (std::abs(velLocal.z()) <= velocityRedline || m_state.gear < Gear::GEAR_1) {
                    if (!lostGrip) {
                        handbrakeAccumulator = 0;
                    }
                } else {
                    // Over speed limit for this gear - lost grip
                    velocityRedline = m_perf.engineRedlineRPM * GearRPMToVelocity(m_state.gear);
                    force = (velocityRedline / std::abs(velLocal.z())) * force;
                    lostGrip = true;
                    handbrakeAccumulator = IncrementHandbrakeAccumulator();
                }

            } else if (rpmTargetWheelsDiff == 0) {
                // Cruising at target RPM
                rpm = std::max(rpmFromWheelsVal, m_perf.engineMinRPM);
                force = drag;

            } else {
                // Accelerating (rpmTargetWheelsDiff > 0)
                if (rpmDiff <= 200) {
                    if (rpmDiff < -300) {
                        rpm += 40;
                    } else {
                        rpm = std::max(rpmFromWheelsVal, m_perf.engineMinRPM);
                    }
                } else {
                    rpm -= 40;
                }
                rpm = std::max(std::min(targetRPM, rpm), m_perf.engineMinRPM);

                float slipFactor = std::abs(m_state.slipAngle) * 0.6f + 1.0f;
                slipFactor = std::min(slipFactor, 1.5f);
                force = force * m_state.throttle * slipFactor;
            }

            // Low engine RPM check
            if (rpm < m_perf.engineMinRPM) {
                force = TractionPowertrain(m_perf.engineMinRPM);
                rpm = m_perf.engineMinRPM;
            }

        } else {
            // Step 3b: Airborne or neutral - adjust RPM toward target

            // Airborne target RPM adjustment
            if (!m_state.hasContactWithGround) {
                targetRPM += m_perf.shiftBlipRPM[static_cast<int>(Gear::GEAR_1)];
            }

            // RPM adjustment when not in gear
            if (rpm < targetRPM && m_state.gearShiftCounter == 0) {
                rpm += 250.0f;
                targetRPM = std::min(rpm, targetRPM);
                targetRPM = std::max(targetRPM, m_perf.engineMinRPM);
                rpm = targetRPM;
            } else {
                if (m_state.gearShiftCounter == 0) {
                    if (!(rpm < targetRPM)) {
                        rpm -= 150.0f;
                        rpm = std::max({targetRPM, rpm, m_perf.engineMinRPM});
                    }
                } else if (!m_state.shiftedDown) {
                    // Upshift - RPM drops
                    int rpmAdjust = -50;
                    if (m_perf.gearShiftDelay < 5) {
                        rpmAdjust = -75;
                    }
                    if (aboveRedline) {
                        rpmAdjust *= 2;
                    }
                    rpm -= rpmAdjust;
                    rpm = std::max(rpm, m_perf.engineMinRPM);
                } else {
                    // Downshift - RPM rises (blip)
                    if (m_state.throttle > 0) {
                        float rpmAdjust = m_perf.brakeBlipRPM[static_cast<int>(m_state.gear)];
                        if (m_state.brake < (65.0f / 255.0f)) {
                            rpmAdjust = m_perf.shiftBlipRPM[static_cast<int>(m_state.gear)];
                        }
                        rpm += rpmAdjust;
                    }
                    rpm = std::clamp(rpm, m_perf.engineMinRPM, m_perf.engineRedlineRPM);
                }
            }
        }

        // Step 4: Postprocess - apply drag
        if (m_state.hasContactWithGround) {
            if (velLocal.z() > 0.0f) {
                force = force - drag;
            } else {
                force = drag * 2.0f + force;
            }
        } else {
            if (velLocal.z() >= 0.0f) {
                force = -drag * 0.5f;
            } else {
                force = drag;
            }
        }

        m_state.rpm = rpm;
        m_state.tractionForce = force;
        m_state.lostGrip = lostGrip;
        m_state.handbrakeAccumulator = handbrakeAccumulator;
    }
} // namespace OpenNFS