#include "RacerAgent.h"

#include <glm/gtx/vector_angle.hpp>

namespace OpenNFS {
    // TODO: Read this from file
    char const *RACER_NAMES[23] = {"DumbPanda",       "Spark198rus", "Keiiko",    "N/A",       "Patas De Pavo", "Dopamine Flint", "Oh Hansssss", "scaryred24",
                                   "MaximilianVeers", "Keith",       "AJ_Lethal", "Sirius-R",  "Ewil",          "Zipper",         "heyitsleo",   "MADMAN_nfs",
                                   "Wild One",        "Gotcha",      "Mulligan",  "Lead Foot", "Ace",           "Dead Beat",      "Ram Rod"};

    RacerAgent::RacerAgent(uint16_t racerID, const std::shared_ptr<Car> &car, const std::shared_ptr<Track> &raceTrack) : CarAgent(AgentType::RACING, car, raceTrack) {
        name          = RACER_NAMES[racerID];
        this->vehicle = std::make_shared<Car>(car->assetData, car->tag, car->id);

        // TODO: DEBUG! Set a low max speed.
        this->vehicle->vehicleProperties.maxSpeed = 100.f;
    }

    void RacerAgent::Simulate() {
        // Update data required for track physics update
        this->_UpdateNearestTrackblock();
        this->_UpdateNearestVroad();

        switch (m_mode) {
        case FollowTrack:
            this->_FollowTrack();
            break;
        case NeuralNet:
            this->_UseNeuralNetAI();
            break;
        case Primitive:
            this->_UsePrimitiveAI();
            break;
        }

        // If during simulation, car flips, reset.
        if ((vehicle->rangefinderInfo.upDistance <= 0.1f || vehicle->rangefinderInfo.downDistance > 1.f ||
             vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_RAY] < 0.25f) &&
            m_ticksAlive > 5000) {
            m_ticksAlive = 0;
            ResetToVroad(m_nearestVroadID, 0.f);
        } else {
            ++m_ticksAlive;
        }
    }
    void RacerAgent::_FollowTrack() {
        glm::vec3 target = m_track->rawTrack->virtualRoad[(m_nearestVroadID + 10) % m_track->virtualRoad.size()].position;
        float angle      = glm::orientedAngle(glm::normalize(Utils::bulletToGlm(this->vehicle->GetVehicle()->getForwardVector())),
                                              glm::normalize(target - this->vehicle->carBodyModel.position), glm::vec3(0, 1, 0));
        // vehicle->ApplyAbsoluteSteerAngle(angle);
        if (angle < -0.15f) {
            vehicle->ApplySteeringRight(true);
        } else if (angle > 0.15f) {
            vehicle->ApplySteeringLeft(true);
        } else {
            vehicle->ApplySteeringRight(false);
            vehicle->ApplySteeringLeft(false);
        }
        vehicle->ApplyAccelerationForce(true, false);
    }

    void RacerAgent::_UsePrimitiveAI() {
        vehicle->ApplyAccelerationForce(true, false);
        vehicle->ApplySteeringLeft(vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_RIGHT_RAY] < 1.f);
        vehicle->ApplySteeringRight(vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_LEFT_RAY] < 1.f);
    }

    void RacerAgent::_UseNeuralNetAI() {
        // Use maximum from front 3 sensors, as per Luigi Cardamone
        float maxForwardDistance =
          std::max({vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_RAY], vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_LEFT_RAY],
                    vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_RIGHT_RAY]});
        // Feed car speed into network so NN can regulate speed
        float carSpeed = vehicle->GetVehicle()->getCurrentSpeedKmHour();

        // All inputs roughly between 0 and 5. Speed/10 to bring it into line.
        // -90, -60, -30, maxForwardDistance {-10, 0, 10}, 30, 60, 90, currentSpeed/10.f
        std::vector<double> networkInputs  = {vehicle->rangefinderInfo.rangefinders[RayDirection::LEFT_RAY],
                                              vehicle->rangefinderInfo.rangefinders[3],
                                              vehicle->rangefinderInfo.rangefinders[6],
                                              maxForwardDistance,
                                              vehicle->rangefinderInfo.rangefinders[12],
                                              vehicle->rangefinderInfo.rangefinders[15],
                                              vehicle->rangefinderInfo.rangefinders[RayDirection::RIGHT_RAY],
                                              carSpeed / 10.f};
        std::vector<double> networkOutputs = {0, 0, 0, 0};

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
