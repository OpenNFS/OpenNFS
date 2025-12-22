#include "RacerAgent.h"

#include "../../Physics/Car.h"
#include "lib/glm/glm/gtx/vector_angle.hpp"

namespace OpenNFS {
    // TODO: Read this from file
    char const *RACER_NAMES[23] = {
        "DumbPanda",       "Spark198rus", "Keiiko",    "N/A",       "Patas De Pavo", "Dopamine Flint", "Oh Hansssss", "scaryred24",
        "MaximilianVeers", "Keith",       "AJ_Lethal", "Sirius-R",  "Ewil",          "Zipper",         "heyitsleo",   "MADMAN_nfs",
        "Wild One",        "Gotcha",      "Mulligan",  "Lead Foot", "Ace",           "Dead Beat",      "Ram Rod"};

    RacerAgent::RacerAgent(uint16_t const racerID, std::shared_ptr<Car> const &car, std::shared_ptr<Track> const &raceTrack)
        : CarAgent(AgentType::RACING, car, raceTrack), m_racerID(racerID) {
        name = RACER_NAMES[racerID];
        this->vehicle = std::make_shared<Car>(car->assetData);

        // TODO: DEBUG! Set a low max speed.
        this->vehicle->assetData.physicsData.maxSpeed = 100.f;
    }

    void RacerAgent::Simulate() {
        // Update data required for track physics update
        this->_UpdateNearestTrackblock();
        this->_UpdateNearestVroad();

        switch (m_mode) {
        case NeuralNet:
            this->_UseNeuralNetAI();
            break;
        case Primitive:
            this->_UsePrimitiveAI();
            break;
        }

        bool const going_backwards{m_lastTrackBlockID > m_nearestTrackblockID};
        bool const no_progress{m_ticksInBlock > kBlockTickLimit};
        bool const upside_down{(vehicle->rangefinderInfo.upDistance <= 0.1f && vehicle->rangefinderInfo.downDistance > 1.f)};
        if (going_backwards) {
            ++m_ticksGoingBackwards;
        }

        // If during simulation, car flips, reset.
        if (upside_down || no_progress || (going_backwards && m_ticksGoingBackwards > 20)) {
            LOG(DEBUG) << "Racer ID: " << m_racerID << " no longer making forward progress, resetting to VRoad ID: " << m_nearestVroadID;
            m_ticksInBlock = 0;
            m_lastTrackBlockID = m_nearestTrackblockID;
            m_ticksGoingBackwards = 0;
            ResetToVroad(m_nearestVroadID, Utils::RandomFloat(-.5f, .5f));
        }

        if (m_nearestTrackblockID >= m_lastTrackBlockID) {
            m_ticksGoingBackwards = 0;
        }

        if (m_lastTrackBlockID != m_nearestTrackblockID) {
            m_lastTrackBlockID = m_nearestTrackblockID;
            m_ticksInBlock = 0;
        } else {
            ++m_ticksInBlock;
        }
    }

    uint32_t RacerAgent::_CarSpeedToLookahead(float const carSpeed) const {
        uint32_t const carSpeedRatio{static_cast<uint32_t>((carSpeed / vehicle->assetData.physicsData.maxSpeed) * 10)};
        uint32_t const offset{1 + (carSpeedRatio * 3)};
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
        uint32_t const futureVroad{_CarSpeedToLookahead(speed)};
        glm::vec3 const target{m_track.virtualRoad[(futureVroad) % m_track.virtualRoad.size()].position};
        float const angle{glm::orientedAngle(glm::normalize(Utils::bulletToGlm(this->vehicle->GetVehicle()->getForwardVector())),
                                             glm::normalize(target - this->vehicle->carBodyModel.position), glm::vec3(0, 1, 0))};
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
