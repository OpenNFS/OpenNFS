#include "RacerAgent.h"

// TODO: Read this from file
char const *RACER_NAMES[23] =
        {
                "DumbPanda",
                "Spark198rus",
                "Keiiko",
                "N/A",
                "Patas De Pavo",
                "Dopamine Flint",
                "Oh Hansssss",
                "scaryred24",
                "MaximilianVeers",
                "Keith",
                "AJ_Lethal",
                "Sirius-R",
                "Ewil",
                "Zipper",
                "heyitsleo",
                "MADMAN_nfs",
                "Wild One",
                "Gotcha",
                "Mulligan",
                "Lead Foot",
                "Ace",
                "Dead Beat",
                "Ram Rod"
        };

RacerAgent::RacerAgent(uint16_t racerID, const std::string &networkPath, std::shared_ptr<Car> car, std::shared_ptr<ONFSTrack> raceTrack) :
CarAgent(AgentType::RACING, car, raceTrack)
{
    if (boost::filesystem::exists(networkPath))
    {
        raceNet.import_fromfile(networkPath);
    }
    else
    {
        LOG(WARNING) << "AI Neural network couldn't be loaded from " << BEST_NETWORK_PATH << ", randomising weights";
    }
    name = RACER_NAMES[racerID];
    this->vehicle = std::make_shared<Car>(car->assetData, car->tag, car->id);
}

void RacerAgent::Simulate()
{
    // Update data required for track physics update
    this->_UpdateNearestTrackblock();
    this->_UpdateNearestVroad();

    // If during simulation, car flips, reset. Not during training, or for player!
    if ((vehicle->rangefinderInfo.upDistance <= 0.1f || vehicle->rangefinderInfo.downDistance > 1.f || vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_RAY] < 0.25f))
    {
        ResetToVroad(m_nearestVroadID, 0.f);
    }

    // Use maximum from front 3 sensors, as per Luigi Cardamone
    float maxForwardDistance = std::max({vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_RAY],
                                         vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_LEFT_RAY],
                                         vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_RIGHT_RAY]});
    // Feed car speed into network so NN can regulate speed
    float carSpeed = vehicle->GetVehicle()->getCurrentSpeedKmHour();

    // All inputs roughly between 0 and 5. Speed/10 to bring it into line.
    // -90, -60, -30, maxForwardDistance {-10, 0, 10}, 30, 60, 90, currentSpeed/10.f
    std::vector<double> networkInputs = {vehicle->rangefinderInfo.rangefinders[RayDirection::LEFT_RAY], vehicle->rangefinderInfo.rangefinders[3], vehicle->rangefinderInfo.rangefinders[6], maxForwardDistance,
                                         vehicle->rangefinderInfo.rangefinders[12], vehicle->rangefinderInfo.rangefinders[15], vehicle->rangefinderInfo.rangefinders[RayDirection::RIGHT_RAY], carSpeed / 10.f};
    std::vector<double> networkOutputs = {0, 0, 0, 0};

    // Inference on the network
    raceNet.evaluate(networkInputs, networkOutputs);

    // Control the vehicle with the neural network outputs
    vehicle->ApplyAccelerationForce(networkOutputs[0] > 0.1f, false);
    vehicle->ApplyBrakingForce(networkOutputs[1] > 0.1f);
    //car->applyAbsoluteSteerAngle(networkOutputs[2]);
    // Mutex steering
    vehicle->ApplySteeringLeft(networkOutputs[2] > 0.1f && networkOutputs[3] < 0.1f);
    vehicle->ApplySteeringRight(networkOutputs[3] > 0.1f && networkOutputs[2] < 0.1f);
}
