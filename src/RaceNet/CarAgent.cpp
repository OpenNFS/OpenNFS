//
// Created by amrik on 28/02/19.
//

#include "CarAgent.h"



CarAgent::CarAgent(uint16_t populationID, const shared_ptr<Car> &trainingCar, const shared_ptr<ONFSTrack> &trainingTrack) : track(trainingTrack) {
    this->populationID = populationID;
    name = "TrainingAgent" + std::to_string(populationID);
    fitness = 0;
    tickCount = 0;
    dead = false;

    this->car = std::make_shared<Car>(trainingCar->all_models, trainingCar->tag, trainingCar->name);
    this->car->colour = glm::vec3(Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f));
}

CarAgent::CarAgent(const std::string &racerName, const std::string &networkPath, const shared_ptr<Car> &car) {
    if (boost::filesystem::exists(networkPath)) {
        raceNet.import_fromfile(networkPath);
    } else {
        LOG(WARNING) << "AI Neural network couldn't be loaded from " << BEST_NETWORK_PATH << ", randomising weights";
    }
    name = racerName;
    this->car = std::make_shared<Car>(car->all_models, car->tag, car->name);
}

void CarAgent::resetToVroad(uint32_t trackBlockIndex, std::shared_ptr<ONFSTrack> &track, std::shared_ptr<Car> &car) {
    glm::vec3 vroadPoint;
    glm::quat carOrientation;

    if (track->tag == NFS_3 || track->tag == NFS_4) {
        // Can move this by trk[trackBlockIndex].nodePositions
        uint32_t nodeNumber = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->trk[trackBlockIndex].nStartPos;

        glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
        COLVROAD resetVroad = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[nodeNumber];
        vroadPoint = (rotationMatrix * TrackUtils::pointToVec(resetVroad.refPt)) / 65536.f;
        vroadPoint /= 10.f;
        vroadPoint.y += 0.2;

        rotationMatrix = glm::normalize(glm::quat(glm::vec3(SIMD_PI / 2, 0, 0)));
        glm::vec3 forward = TrackUtils::pointToVec(resetVroad.forward) * rotationMatrix;
        glm::vec3 normal = TrackUtils::pointToVec(resetVroad.normal) * rotationMatrix;
        carOrientation = glm::conjugate(glm::toQuat(
                glm::lookAt(vroadPoint,
                            vroadPoint + forward,
                            normal
                )
        ));
    } else {
        vroadPoint = TrackUtils::pointToVec(track->trackBlocks[trackBlockIndex].center);
        vroadPoint.y += 0.2;
        carOrientation = glm::quat(2, 0, 0, 1);
    }

    // Go and find the Vroad Data to reset to
    car->resetCar(vroadPoint, carOrientation);
}

bool CarAgent::isWinner() {
    fitness = evaluateFitness();
    return fitness > 5000;
}

void CarAgent::reset(){
    resetToVroad(1, track, car);
}

void CarAgent::simulate() {
    if (dead){
        return ;
    }

    std::vector<double> raycastInputs;
    std::vector<double> networkOutputs;

    raycastInputs = {car->leftDistance, car->rightDistance, car->forwardDistance};
    networkOutputs = {0, 0, 0};

    raceNet.evaluate(raycastInputs, networkOutputs);

    car->applyAccelerationForce(false, networkOutputs[0] > 0.0f);
    car->applySteeringLeft(networkOutputs[1] > 0.0f);
    car->applySteeringRight(networkOutputs[2] > 0.0f);

    // Speculatively calculate where we're gonna end up
    uint32_t new_fitness = evaluateFitness();

    if (new_fitness > fitness){
        tickCount = 0;
        fitness = new_fitness;
    }

    ++tickCount;
    if (tickCount > STALE_TICK_COUNT){
        dead = true;
    }
}

uint32_t CarAgent::evaluateFitness() {
    uint32_t nVroad = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroadHead.nrec;

    uint32_t closestVroadID = 0;
    float lowestDistanceSqr = FLT_MAX;
    for (uint32_t vroad_Idx = 0; vroad_Idx < nVroad; ++vroad_Idx) {
        INTPT refPt = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[vroad_Idx].refPt;
        glm::vec3 vroadPoint = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))) *
                               glm::vec3((refPt.x / 65536.0f) / 10.f, ((refPt.y / 65536.0f) / 10.f),
                                         (refPt.z / 65536.0f) / 10.f);

        float distanceSqr = glm::length2(glm::distance(car->car_body_model.position, vroadPoint));
        if (distanceSqr < lowestDistanceSqr) {
            closestVroadID = vroad_Idx;
            lowestDistanceSqr = distanceSqr;
        }
    }
    // Return a number corresponding to the distance driven
    return closestVroadID;
}



