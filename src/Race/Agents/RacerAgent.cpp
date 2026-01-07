#include "RacerAgent.h"

#include "../../Physics/Car.h"
#include "lib/glm/glm/gtx/vector_angle.hpp"

namespace OpenNFS {
    RacerAgent::RacerAgent(RacerData const &racerData, std::shared_ptr<Car> const &car, std::shared_ptr<Track> const &raceTrack)
        : CarAgent(AgentType::AI, car, raceTrack) {
        name = racerData.name;
        this->vehicle = std::make_shared<Car>(car->assetData);

        // TODO: DEBUG! Set a low max speed.
        this->vehicle->assetData.physicsData.maxSpeed = 100.f;
    }

    void RacerAgent::Simulate() {
        // Update data required for track physics update
        this->_UpdateNearestTrackblock();
        this->_UpdateNearestVroad();

        // FSM: State transitions and logic
        switch (m_state) {
        case RacerState::RESETTING:
            // Reset position and state
            ResetToVroad(m_nearestVroadID, Utils::RandomFloat(-.5f, .5f));
            m_nextState = RacerState::TRACK_FOLLOWING;
            m_ticksInBlock = 0;
            m_lastTrackBlockID = m_nearestTrackblockID;
            m_ticksStuck = 0;
            m_ticksReversing = 0;
            m_reverseAttempts = 0;
            break;
        case RacerState::TRACK_FOLLOWING: {
            bool const goingBackwards{m_lastTrackBlockID > m_nearestTrackblockID};
            bool const noProgress{m_ticksInBlock > kBlockTickLimit};
            bool const upsideDown{vehicle->rangefinderInfo.upDistance <= 0.1f && vehicle->rangefinderInfo.downDistance > 1.f};

            // Check critical failure conditions that require position reset
            bool const stuckAfterAllAttempts{m_ticksStuck >= kStuckTicksThreshold && m_reverseAttempts >= kMaxReverseAttempts};

            // Track if moving slowly (potential stuck situation)
            if (noProgress) {
                ++m_ticksStuck;

                // Transition to STUCK_REVERSING if stuck threshold reached and have attempts left
                if (m_ticksStuck >= kStuckTicksThreshold && m_reverseAttempts < kMaxReverseAttempts) {
                    m_nextState = RacerState::STUCK_REVERSING;
                    m_ticksReversing = 0;
                    ++m_reverseAttempts;
                }
            } else if (upsideDown || goingBackwards || stuckAfterAllAttempts) {
                m_nextState = RacerState::RESETTING;
            } else {
                // Car is moving well, reset stuck tracking
                m_ticksStuck = 0;
                m_reverseAttempts = 0;
            }
        } break;
        case RacerState::STUCK_REVERSING:
            ++m_ticksReversing;

            // Check if we've reversed long enough
            if (m_ticksReversing >= kReverseTicksLimit) {
                m_nextState = RacerState::TRACK_FOLLOWING;
                m_ticksReversing = 0;
                m_ticksStuck = 0; // Give forward motion a fresh chance
            }
            break;
        }

        // Execute AI based on current state
        switch (m_mode) {
        case NeuralNet:
            this->_UseNeuralNetAI();
            break;
        case Primitive:
            this->_UsePrimitiveAI();
            break;
        }

        // Track progress through track blocks
        if (m_lastTrackBlockID != m_nearestTrackblockID) {
            m_lastTrackBlockID = m_nearestTrackblockID;
            m_ticksInBlock = 0;
        } else {
            ++m_ticksInBlock;
        }

        if (m_nextState != m_state) {
            LOG(DEBUG) << "Racer ID: " << m_racerID << " State Update: [" << magic_enum::enum_name(m_state) << "] -> ["
                       << magic_enum::enum_name(m_nextState) << "]";
            m_state = m_nextState;
        }
    }

    RacerState RacerAgent::State() const {
        return m_state;
    }

    uint32_t RacerAgent::_CarSpeedToLookahead(float const carSpeed) const {
        uint32_t const carSpeedRatio{static_cast<uint32_t>((carSpeed / vehicle->assetData.physicsData.maxSpeed) * 10)};
        uint32_t const offset{2 + (carSpeedRatio * 4)};
        return m_nearestVroadID + offset;
    }

    float RacerAgent::_CarSpeedToSteeringDamper(float const carSpeed) const {
        float const carSpeedRatio{carSpeed / vehicle->assetData.physicsData.maxSpeed}; // 0 -> 1.0 (0.3 max, in practice)
        // At Max speed: 0.5-0.3 = 0.2f. At Min speed: 0.5f - 0.f = 0.5f.
        // Min steering damper = 0.1f
        return std::min(kSteeringDamper - carSpeedRatio, 0.1f);
    }

    void RacerAgent::_UsePrimitiveAI() {
        auto const speed{vehicle->GetVehicle()->getCurrentSpeedKmHour()};

        // State-based behavior
        switch (m_state) {
        case RacerState::RESETTING:
            break;
        case RacerState::STUCK_REVERSING:
            // Apply reverse with alternated max lock left/right per attempt number to wiggle out
            {
                float const reverseSteerAngle{(m_reverseAttempts % 2) ? 0.5f : -0.5f};
                vehicle->ApplyAbsoluteSteerAngle(reverseSteerAngle);
                vehicle->ApplyAccelerationForce(false, true); // Accelerate in reverse
                vehicle->ApplyBrakingForce(false);
            }
            break;

        case RacerState::TRACK_FOLLOWING:
            // Normal forward AI behavior
            {
                m_futureVroadID = _CarSpeedToLookahead(speed);
                targetVroadPosition = m_track->virtualRoad[(m_futureVroadID) % m_track->virtualRoad.size()].position;
                float const angle{glm::orientedAngle(glm::normalize(Utils::bulletToGlm(this->vehicle->GetVehicle()->getForwardVector())),
                                                     glm::normalize(targetVroadPosition - this->vehicle->carBodyModel.position),
                                                     glm::vec3(0, 1, 0))};
                m_steeringAngle = (_CarSpeedToSteeringDamper(speed) * angle);
                float const steeringDifference{std::abs(m_steeringAngle - angle)};
                bool const accelerate{steeringDifference < 0.15f || speed <= kMinSpeed};
                bool const overrideLeft{vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_RIGHT_RAY] < 1.f};
                bool const overrideRight{vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_LEFT_RAY] < 1.f};
                if (overrideLeft || overrideRight) {
                    vehicle->ApplySteeringLeft(overrideLeft);
                    vehicle->ApplySteeringRight(overrideRight);
                } else {
                    vehicle->ApplyAbsoluteSteerAngle(m_steeringAngle);
                }
                vehicle->ApplyAccelerationForce(accelerate, false);
                vehicle->ApplyBrakingForce(vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_RAY] <= 1.f);
            }
            break;
        }
    }

    void RacerAgent::_UseNeuralNetAI() const {
        // Use maximum from front 3 sensors, as per Luigi Cardamone
        float const maxForwardDistance{std::max({vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_RAY],
                                                 vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_LEFT_RAY],
                                                 vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_RIGHT_RAY]})};
        // Feed car speed into network so NN can regulate speed
        float const carSpeed{vehicle->GetVehicle()->getCurrentSpeedKmHour()};

        // All inputs roughly between 0 and 5. Speed/10 to bring it into line.
        // -90, -60, -30, maxForwardDistance {-10, 0, 10}, 30, 60, 90, currentSpeed/10.f
        std::vector<double> networkInputs = {vehicle->rangefinderInfo.rangefinders[RayDirection::LEFT_RAY],
                                             vehicle->rangefinderInfo.rangefinders[3],
                                             vehicle->rangefinderInfo.rangefinders[6],
                                             maxForwardDistance,
                                             vehicle->rangefinderInfo.rangefinders[12],
                                             vehicle->rangefinderInfo.rangefinders[15],
                                             vehicle->rangefinderInfo.rangefinders[RayDirection::RIGHT_RAY],
                                             carSpeed / 10.f};
        std::vector<double> const networkOutputs = {0, 0, 0, 0};

        // Inference on the network
        // raceNet.evaluate(networkInputs, networkOutputs);

        // Control the vehicle with the neural network outputs
        vehicle->ApplyAccelerationForce(networkOutputs[0] > 0.1f, false);
        vehicle->ApplyBrakingForce(networkOutputs[1] > 0.1f);
        // car->applyAbsoluteSteerAngle(networkOutputs[2]);
        // Mutex steering
        vehicle->ApplySteeringLeft(networkOutputs[2] > 0.1f && networkOutputs[3] < 0.1f);
        vehicle->ApplySteeringRight(networkOutputs[3] > 0.1f && networkOutputs[2] < 0.1f);
    }
} // namespace OpenNFS
