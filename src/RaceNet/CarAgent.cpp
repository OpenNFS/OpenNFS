#include "CarAgent.h"

CarAgent::CarAgent(uint16_t populationID, std::shared_ptr<Car> trainingCar, std::shared_ptr<ONFSTrack> trainingTrack) : car(std::make_shared<Car>(trainingCar->data, trainingCar->tag, trainingCar->id)), track(trainingTrack), name("TrainingAgent" + std::to_string(populationID)) {
    this->populationID = populationID;
    training = true;
    fitness = 0;
    tickCount = 0;
    dead = false;
    this->car->colour = glm::vec3(Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f));
}

CarAgent::CarAgent(const std::string &racerName, const std::string &networkPath, std::shared_ptr<Car> car, std::shared_ptr<ONFSTrack> trainingTrack) : track(trainingTrack) {
    if (boost::filesystem::exists(networkPath)) {
        raceNet.import_fromfile(networkPath);
    } else {
        LOG(WARNING) << "AI Neural network couldn't be loaded from " << BEST_NETWORK_PATH << ", randomising weights";
    }
    name = racerName;
    this->car = std::make_shared<Car>(car->data, car->tag, car->id);
}

void CarAgent::resetToVroad(int trackBlockIndex, int posIndex, float offset, const std::shared_ptr<ONFSTrack> &track, const std::shared_ptr<Car> &car) {
    glm::vec3 vroadPoint;
    glm::quat carOrientation;

    ASSERT(offset <= 1.f, "Cannot reset to offset larger than +- 1.f on VROAD (Will spawn off track!)");

    if (track->tag == NFS_3 || track->tag == NFS_4) {
        // Can move this by trk[trackBlockIndex].nodePositions
        uint32_t nodeNumber = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->trk[trackBlockIndex].nStartPos;
        int nPositions = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->trk[trackBlockIndex].nPositions;
        if (posIndex <= nPositions){
            nodeNumber += posIndex;
        } else {
            // Advance the trackblock until we can get to posIndex
            int nExtra = posIndex - nPositions;
            while(true){
                nodeNumber = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->trk[++trackBlockIndex].nStartPos;
                nPositions = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->trk[trackBlockIndex].nPositions;
                if(nExtra < nPositions){
                    nodeNumber += nExtra;
                    break;
                } else {
                    nExtra -= nPositions;
                }
            }
        }
        glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
        COLVROAD resetVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[nodeNumber];
        vroadPoint = (rotationMatrix * Utils::PointToVec(resetVroad.refPt)) / 65536.f;
        vroadPoint /= 10.f;
        vroadPoint.y += 0.2;

        // Get VROAD right vector
        glm::vec3 curVroadRightVec = rotationMatrix * glm::vec3(resetVroad.right.x / 128.f, resetVroad.right.y / 128.f, resetVroad.right.z / 128.f);
        vroadPoint += offset * curVroadRightVec;

        rotationMatrix = glm::normalize(glm::quat(glm::vec3(SIMD_PI / 2, 0, 0)));
        glm::vec3 forward = Utils::PointToVec(resetVroad.forward) * rotationMatrix;
        glm::vec3 normal = Utils::PointToVec(resetVroad.normal) * rotationMatrix;
        carOrientation = glm::conjugate(glm::toQuat(
                glm::lookAt(vroadPoint,
                            vroadPoint - forward,
                            normal
                )
        ));
    } else {
        vroadPoint = Utils::PointToVec(track->trackBlocks[trackBlockIndex].center);
        vroadPoint.y += 0.2;
        carOrientation = glm::quat(2, 0, 0, 1);
    }

    // Go and find the Vroad Data to reset to
    car->resetCar(vroadPoint, carOrientation);
}

void CarAgent::resetToVroad(int vroadIndex, float offset, const std::shared_ptr<ONFSTrack> &track, const std::shared_ptr<Car> &car)  {
    glm::vec3 vroadPoint;
    glm::quat carOrientation;

    ASSERT(offset <= 1.f, "Cannot reset to offset larger than +- 1.f on VROAD (Will spawn off track!)");

    if (track->tag == NFS_3 || track->tag == NFS_4) {
        uint32_t nVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroadHead.nrec;
        ASSERT(vroadIndex < nVroad, "Requested reset to vroad index: " << vroadIndex << " outside of nVroad: " << nVroad);

        glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
        COLVROAD resetVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[vroadIndex];
        vroadPoint = (rotationMatrix * Utils::PointToVec(resetVroad.refPt)) / 65536.f;
        vroadPoint /= 10.f;
        vroadPoint.y += 0.2;

        // Get VROAD right vector
        glm::vec3 curVroadRightVec = rotationMatrix * glm::vec3(resetVroad.right.x / 128.f, resetVroad.right.y / 128.f, resetVroad.right.z / 128.f);
        vroadPoint += offset * curVroadRightVec;

        rotationMatrix = glm::normalize(glm::quat(glm::vec3(SIMD_PI / 2, 0, 0)));
        glm::vec3 forward = Utils::PointToVec(resetVroad.forward) * rotationMatrix;
        glm::vec3 normal = Utils::PointToVec(resetVroad.normal) * rotationMatrix;
        carOrientation = glm::conjugate(glm::toQuat(
                glm::lookAt(vroadPoint,
                            vroadPoint - forward,
                            normal
                )
        ));
    } else {
        ASSERT(false, "Vroad Index based reset not available outside NFS3 for now.");
    }

    // Go and find the Vroad Data to reset to
    car->resetCar(vroadPoint, carOrientation);
}

int CarAgent::getClosestVroad(const std::shared_ptr<Car> &car, const std::shared_ptr<ONFSTrack> &track) {
    int closestVroadID = 0;
    if(track-> tag == NFS_3 || track->tag == NFS_4){
        uint32_t nVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroadHead.nrec;


        float lowestDistance = FLT_MAX;
        for (int vroad_Idx = 0; vroad_Idx < nVroad; ++vroad_Idx) {
            INTPT refPt = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[vroad_Idx].refPt;
            glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
            glm::vec3 vroadPoint = rotationMatrix * glm::vec3((refPt.x / 65536.0f) / 10.f, ((refPt.y / 65536.0f) / 10.f), (refPt.z / 65536.0f) / 10.f);

            float distance = glm::distance(car->rightFrontWheelModel.position, vroadPoint);
            if (distance < lowestDistance) {
                closestVroadID = vroad_Idx;
                lowestDistance = distance;
            }
        }
    } else {
        // TODO: Implement this for NFS_2 and 3_PS1
        closestVroadID = 0;
    }

    // Return a number corresponding to the distance driven
    return closestVroadID;

}

int CarAgent::evaluateFitness(int vroadPosition){
    // F = C1 − Tout + C2 · s¯+ d, where Tout is the number of game tics the car is outside the track;
    // ¯s is the average speed (meters for game tic) during the evaluation;
    // d is the distance (meters) raced by the car during the evaluation;
    // C1 and C1 are two constants introduced respectively to make sure that the fitness is positive
    // and to scale the average speed term (both C1 and C2 have been empirically set to 1000 in all the experiment reported) Luigi Cardamone
    int c1, c2;
    c1 = c2 = 1000;

    int timeOutsideVroad = tickCount - insideVroadCount;
    //int fitness = c1 - timeOutsideVroad + (int) (c2 * averageSpeed) + closestVroad;

    int fitness = (int) pow(vroadPosition, 1);

    return fitness;
}

bool CarAgent::isWinner() {
    if(droveBack || dead) return false;

    fitness = evaluateFitness(getClosestVroad(car, track));

    int nVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroadHead.nrec;

    // Have won if have made it near to end of track
    return fitness > pow(nVroad - 30, 1);
}

void CarAgent::reset(){
    resetToVroad(0, 0.f, track, car);
}

void CarAgent::simulate() {
    // Track our old position
    static int vroadPosition;

    // If the agent is dead, there is no need to simulate it.
    if (dead && training){
        return ;
    }

    // If during simulation, car flips, reset. Not during training!
    if(!training && ((car->upDistance <= 0.1f || car->downDistance > 1.f || car->rangefinders[Car::FORWARD_RAY] < 0.25f))){
        resetToVroad(getClosestVroad(car, track), 0.f, track, car);
    }

    // Use maximum from front 3 sensors, as per Luigi Cardamone
    float maxForwardDistance = std::max({car->rangefinders[Car::FORWARD_RAY], car->rangefinders[Car::FORWARD_LEFT_RAY], car->rangefinders[Car::FORWARD_RIGHT_RAY]});
    // Feed car speed into network so NN can regulate speed
    float carSpeed = car->m_vehicle->getCurrentSpeedKmHour();

    // All inputs roughly between 0 and 5. Speed/10 to bring it into line.
    // -90, -60, -30, maxForwardDistance {-10, 0, 10}, 30, 60, 90, currentSpeed/10.f
    std::vector<double> networkInputs = {car->rangefinders[Car::LEFT_RAY], car->rangefinders[3], car->rangefinders[6], maxForwardDistance, car->rangefinders[12], car->rangefinders[15], car->rangefinders[Car::RIGHT_RAY], carSpeed/ 10.f};
    std::vector<double> networkOutputs = {0, 0, 0, 0};

    // Inference on the network
    raceNet.evaluate(networkInputs, networkOutputs);

    // Control the vehicle with the neural network outputs
    car->applyAccelerationForce(networkOutputs[0] > 0.1f, false);
    car->applyBrakingForce(networkOutputs[1] > 0.1f);
    //car->applyAbsoluteSteerAngle(networkOutputs[2]);
    // Mutex steering
    car->applySteeringLeft(networkOutputs[2] > 0.1f && networkOutputs[3] < 0.1f);
    car->applySteeringRight(networkOutputs[3] > 0.1f && networkOutputs[2] < 0.1f);

    if(!training) return;

    // Count how long the car has been inside vroad, to evaluate fitness later
    if(car->upDistance < 0.5f){
        ++insideVroadCount;
    }

    // Update the running average speed of the vehicle for eventual fitness calculation
    averageSpeed += carSpeed;
    averageSpeed /= tickCount + 1;

    // Work out whether fitness is regressing
    int newVroadPosition = getClosestVroad(car, track);

    // If the vroad position jumps this much between ticks, we probably reversed over the start line.
    if (abs(newVroadPosition - vroadPosition) > 100){
        dead = droveBack = true;
        return;
    }

    // If not moved in more than 100 ticks of the game engine, we're dead
    if(abs(newVroadPosition - vroadPosition) == 0 && tickCount > 100){
        dead = true;
        return;
    }

    // Calculate new fitness after moving
    int newFitness = evaluateFitness(vroadPosition);

    // The fitness has increased, set it to the new value and reset the stale tick count
    if (newFitness > fitness){
        tickCount = 0;
        fitness = newFitness;
    }

    // If the fitness has not increased within the STALE_TICK_COUNT, we should die
    if (++tickCount > STALE_TICK_COUNT){
        dead = true;
    }

    // Our current position is the new position
    vroadPosition = newVroadPosition;
}




