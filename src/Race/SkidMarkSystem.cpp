#include "SkidMarkSystem.h"
#include "../Physics/Car.h"
#include "../Physics/RaycastVehicle.h"
#include "Agents/CarAgent.h"

#include <glm/gtc/matrix_transform.hpp>

namespace OpenNFS {

    void WheelTrail::AddSegment(TrailSegment const &segment) {
        size_t const insertIdx = (headIndex + count) % MAX_SEGMENTS;

        if (count < MAX_SEGMENTS) {
            segments[insertIdx] = segment;
            ++count;
        } else {
            // Buffer is full, overwrite oldest
            segments[headIndex] = segment;
            headIndex = (headIndex + 1) % MAX_SEGMENTS;
        }
    }

    void WheelTrail::Clear() {
        headIndex = 0;
        count = 0;
        wasInContact = false;
        lastPosition = glm::vec3(0.0f);
        for (auto &seg : segments) {
            seg.valid = false;
        }
    }

    SkidMarkSystem::SkidMarkSystem() {
        // Reserve space for a reasonable number of vehicles
        m_vehicleTrails.reserve(8);
    }

    void SkidMarkSystem::Clear() {
        for (auto &vehicleTrail : m_vehicleTrails) {
            for (auto &wheelTrail : vehicleTrail.wheelTrails) {
                wheelTrail.Clear();
            }
        }
    }

    // TODO: This is just dummy nonsense for now. Need to read track polyflags and produce a 2D map that we can lookup for surface type.
    SurfaceCategory SkidMarkSystem::MapSurfaceType(int const nfsSurfaceType) {
        switch (nfsSurfaceType) {
            case 0:
                return SurfaceCategory::ASPHALT;
            case 1:
                return SurfaceCategory::DIRT;
            case 2:
                return SurfaceCategory::GRASS;
            case 3:
                return SurfaceCategory::GRAVEL;
            case 4:
                return SurfaceCategory::SNOW;
            case 5:
                return SurfaceCategory::SAND;
            default:
                return SurfaceCategory::UNKNOWN;
        }
    }

    bool SkidMarkSystem::ShouldEmitSkidMark(std::shared_ptr<Car> const &car, int const wheelIndex) const {
        auto *vehicle = car->GetVehicle();
        if (!vehicle) {
            return false;
        }

        auto const &contact = static_cast<RaycastVehicle *>(vehicle)->getWheelContact(wheelIndex);

        // Must be in contact with ground
        if (!contact.isInContact) {
            return false;
        }

        // Check for NFS4 physics model
        auto const *nfs4Physics = car->GetNFS4VehiclePhysics();
        if (nfs4Physics) {
            auto const &state = nfs4Physics->GetState();
            auto const &debug = nfs4Physics->GetDebugData();

            // Handbrake always creates marks on rear wheels
            if (state.handbrakeInput && wheelIndex == Utils::OneOf<REAR_LEFT,REAR_RIGHT>()) {
                return true;
            }

            // Check for loss of grip
            if (state.lostGrip) {
                return true;
            }

            // Check wheel grip
            if (debug.wheels[wheelIndex].grip < m_gripThreshold) {
                return true;
            }

            // Heavy braking
            if (state.brake > 0.8f && nfs4Physics->GetSpeedKMH() > 20.0f) {
                return true;
            }
        } else {
            // Fallback for non-NFS4 physics: check vehicle state
            auto const &vehicleState = car->vehicleState;

            // Handbrake on rear wheels
            if (vehicleState.handbrake && (wheelIndex == 2 || wheelIndex == 3)) {
                return true;
            }

            // Heavy braking
            if (vehicleState.gBrakingForce > 50.0f) {
                return true;
            }
        }

        return false;
    }

    float SkidMarkSystem::CalculateSkidIntensity(std::shared_ptr<Car> const &car, int const wheelIndex) {
        auto const *nfs4Physics = car->GetNFS4VehiclePhysics();
        if (nfs4Physics) {
            auto const &state = nfs4Physics->GetState();
            auto const &debug = nfs4Physics->GetDebugData();

            float intensity = 0.0f;

            // Handbrake contribution
            if (state.handbrakeInput && (wheelIndex == 2 || wheelIndex == 3)) {
                intensity = std::max(intensity, 0.8f);
            }

            // Grip loss contribution
            float const gripLoss = 1.0f - debug.wheels[wheelIndex].grip;
            intensity = std::max(intensity, gripLoss);

            // Brake contribution
            if (state.brake > 0.5f) {
                float const speedFactor = std::min(1.0f, nfs4Physics->GetSpeedKMH() / 100.0f);
                intensity = std::max(intensity, state.brake * speedFactor * 0.7f);
            }

            // Slip angle contribution
            float const slipContrib = std::abs(state.slipAngle) * 2.0f;
            intensity = std::max(intensity, std::min(1.0f, slipContrib));

            return std::clamp(intensity, 0.0f, 1.0f);
        }

        // Fallback for non-NFS4 physics
        auto const &vehicleState = car->vehicleState;

        if (vehicleState.handbrake && (wheelIndex == 2 || wheelIndex == 3)) {
            return 0.8f;
        }

        if (vehicleState.gBrakingForce > 50.0f) {
            return std::min(1.0f, vehicleState.gBrakingForce / 100.0f);
        }

        return 0.5f;
    }

    void SkidMarkSystem::ProcessVehicle(float const deltaTime, std::shared_ptr<CarAgent> const &racer, size_t const vehicleIndex) {
        // Ensure we have enough vehicle trail slots
        while (m_vehicleTrails.size() <= vehicleIndex) {
            VehicleTrails newTrails{};
            newTrails.vehicleID = static_cast<uint32_t>(m_vehicleTrails.size());
            m_vehicleTrails.push_back(newTrails);
        }

        auto &[wheelTrails, vehicleID] = m_vehicleTrails[vehicleIndex];
        auto car = racer->vehicle;
        auto *vehicle = car->GetVehicle();

        if (!vehicle || !car->GetNFS4VehiclePhysics()) {
            return;
        }

        float const tireHalfWidth = (car->assetData.physicsData.wheelWidth / 2) * m_tireHalfWidthMult;

        // Process each wheel
        for (int wheelIdx = 0; wheelIdx < 4; ++wheelIdx) {
            auto &wheelTrail = wheelTrails[wheelIdx];
            auto const &contact = static_cast<RaycastVehicle *>(vehicle)->getWheelContact(wheelIdx);

            bool const shouldEmit = ShouldEmitSkidMark(car, wheelIdx);
            bool const isInContact = contact.isInContact;

            if (shouldEmit && isInContact) {
                // Get wheel position and axle direction
                glm::vec3 contactPos(contact.contactPointWS.x(), contact.contactPointWS.y(), contact.contactPointWS.z());
                glm::vec3 contactNormal(contact.contactNormalWS.x(), contact.contactNormalWS.y(), contact.contactNormalWS.z());
                glm::vec3 wheelAxle(contact.wheelAxleWS.x(), contact.wheelAxleWS.y(), contact.wheelAxleWS.z());

                // Check minimum distance from last segment
                float distFromLast = glm::length(contactPos - wheelTrail.lastPosition);

                if (!wheelTrail.wasInContact || distFromLast >= m_minSegmentDistance) {
                    // Create new segment
                    TrailSegment segment{};

                    // Calculate left and right positions using wheel axle direction
                    glm::vec3 axleNormalized = glm::normalize(wheelAxle);
                    segment.leftPosition = contactPos - axleNormalized * tireHalfWidth;
                    segment.rightPosition = contactPos + axleNormalized * tireHalfWidth;

                    // Add slight height offset above ground
                    segment.leftPosition += contactNormal * m_heightOffset;
                    segment.rightPosition += contactNormal * m_heightOffset;

                    segment.normal = contactNormal;
                    segment.age = 0.0f;
                    segment.intensity = CalculateSkidIntensity(car, wheelIdx);
                    segment.surface = MapSurfaceType(contact.surfaceType);
                    segment.valid = true;

                    wheelTrail.AddSegment(segment);
                    wheelTrail.lastPosition = contactPos;
                }

                wheelTrail.wasInContact = true;
            } else {
                // Not emitting - mark gap in trail
                wheelTrail.wasInContact = false;
            }
        }
    }

    void SkidMarkSystem::AgeSegments(float const deltaTime) {
        for (auto &[wheelTrails, vehicleID] : m_vehicleTrails) {
            for (auto &wheelTrail : wheelTrails) {
                // Count valid segments after aging
                size_t newCount = 0;
                size_t newHead = wheelTrail.headIndex;
                bool foundFirst = false;

                for (size_t i = 0; i < wheelTrail.count; ++i) {
                    size_t const idx = (wheelTrail.headIndex + i) % WheelTrail::MAX_SEGMENTS;
                    auto &segment = wheelTrail.segments[idx];

                    if (segment.valid) {
                        segment.age += deltaTime;

                        if (segment.age >= m_lifetime) {
                            segment.valid = false;
                        } else {
                            if (!foundFirst) {
                                newHead = idx;
                                foundFirst = true;
                            }
                            ++newCount;
                        }
                    }
                }

                wheelTrail.headIndex = newHead;
                wheelTrail.count = newCount;
            }
        }
    }

    void SkidMarkSystem::Update(float const deltaTime, std::vector<std::shared_ptr<CarAgent>> const &racers) {
        // Process each vehicle
        for (size_t i = 0; i < racers.size(); ++i) {
            ProcessVehicle(deltaTime, racers[i], i);
        }

        // Age and cull old segments
        AgeSegments(deltaTime);
    }

} // namespace OpenNFS
