#include "TrainingAgent.h"

TrainingAgent::TrainingAgent(uint16_t populationID, const std::shared_ptr<Car> &trainingCar, const std::shared_ptr<Track> &trainingTrack) :
    CarAgent(AgentType::TRAINING, trainingCar, trainingTrack)
{
    name                                    = "TrainingAgent" + std::to_string(populationID);
    this->populationID                      = populationID;
    fitness                                 = 0;
    m_ticksSpentAlive                       = 0;
    isDead                                  = false;
    this->vehicle->vehicleProperties.colour = glm::vec3(Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f));
}

bool TrainingAgent::IsWinner()
{
    if (m_droveBack || isDead)
    {
        return false;
    }

    fitness = _EvaluateFitness(m_nearestVroadID);

    int nVroad = 0; // boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(m_track->trackData)->col.vroadHead.nrec;

    // Have won if have made it near to end of track
    return fitness > pow(nVroad - 30, 1);
}

void TrainingAgent::Reset()
{
    ResetToVroad(0, 0.f);
}

int TrainingAgent::_EvaluateFitness(int vroadPosition)
{
    // F = C1 − Tout + C2 · s¯+ d, where Tout is the number of game tics the car is outside the track;
    // ¯s is the average speed (meters for game tic) during the evaluation;
    // d is the distance (meters) raced by the car during the evaluation;
    // C1 and C1 are two constants introduced respectively to make sure that the fitness is positive
    // and to scale the average speed term (both C1 and C2 have been empirically set to 1000 in all the experiment reported) Luigi Cardamone
    int c1, c2;
    c1 = c2 = 1000;

    int timeOutsideVroad = m_ticksSpentAlive - ticksInsideVroad;
    // int fitness = c1 - timeOutsideVroad + (int) (c2 * averageSpeed) + closestVroad;

    int fitness = (int) pow(vroadPosition, 1);

    return fitness;
}

void TrainingAgent::Simulate()
{
    // Update data required for track physics update
    this->_UpdateNearestTrackblock();
    this->_UpdateNearestVroad();

    // TrackModel our old position
    static int vroadPosition;

    // If the agent is dead, there is no need to simulate it.
    if (isDead)
    {
        return;
    }

    // If during simulation, car flips, reset. Not during training, or for player!
    if ((vehicle->rangefinderInfo.upDistance <= 0.1f || vehicle->rangefinderInfo.downDistance > 1.f || vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_RAY] < 0.25f))
    {
        ResetToVroad(m_nearestVroadID, 0.f);
    }

    // Use maximum from front 3 sensors, as per Luigi Cardamone
    float maxForwardDistance = std::max({vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_RAY], vehicle->rangefinderInfo.rangefinders[RayDirection::FORWARD_LEFT_RAY],
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
    raceNet.evaluate(networkInputs, networkOutputs);

    // Control the vehicle with the neural network outputs
    vehicle->ApplyAccelerationForce(networkOutputs[0] > 0.1f, false);
    vehicle->ApplyBrakingForce(networkOutputs[1] > 0.1f);
    // car->applyAbsoluteSteerAngle(networkOutputs[2]);
    // Mutex steering
    vehicle->ApplySteeringLeft(networkOutputs[2] > 0.1f && networkOutputs[3] < 0.1f);
    vehicle->ApplySteeringRight(networkOutputs[3] > 0.1f && networkOutputs[2] < 0.1f);

    // Count how long the car has been inside vroad, to evaluate fitness later
    if (vehicle->rangefinderInfo.upDistance < 0.5f)
    {
        ++ticksInsideVroad;
    }

    // Update the running average speed of the vehicle for eventual fitness calculation
    averageSpeed += carSpeed;
    averageSpeed /= m_ticksSpentAlive + 1;

    // Work out whether fitness is regressing
    int newVroadPosition = m_nearestVroadID;

    // If the vroad position jumps this much between ticks, we probably reversed over the start line.
    if (abs(newVroadPosition - vroadPosition) > 100)
    {
        isDead = m_droveBack = true;
        return;
    }

    // If not moved in more than 100 ticks of the game engine, we're dead
    if (abs(newVroadPosition - vroadPosition) == 0 && m_ticksSpentAlive > 100)
    {
        isDead = true;
        return;
    }

    // Calculate new fitness after moving
    int newFitness = _EvaluateFitness(vroadPosition);

    // The fitness has increased, set it to the new value and reset the stale tick count
    if (newFitness > fitness)
    {
        m_ticksSpentAlive = 0;
        fitness           = newFitness;
    }

    // If the fitness has not increased within the STALE_TICK_COUNT, we should die
    if (++m_ticksSpentAlive > STALE_TICK_COUNT)
    {
        isDead = true;
    }

    // Our current position is the new position
    vroadPosition = m_nearestVroadID;
}