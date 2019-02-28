//
// Created by amrik on 28/02/19.
//

#include "CarAgent.h"


bool CarAgent::isWinner() {
    fitness = evaluateFitness();
    return fitness > 5000;
}

void CarAgent::simulate() {
    if (dead){
        return ;
    }

    // Speculatively calculate where we're gonna end up
    double elapsed =  timer.elapsed();
    timer.reset();

    std::vector<double> raycastInputs;
    std::vector<double> networkOutputs;

    raycastInputs = {car->leftDistance, car->rightDistance, car->forwardDistance};
    networkOutputs = {0, 0, 0};

    raceNet.evaluate(raycastInputs, networkOutputs);

    car->applyAccelerationForce(false, networkOutputs[0] > 0.0f);
    car->applySteeringLeft(networkOutputs[1] > 0.0f);
    car->applySteeringRight(networkOutputs[2] > 0.0f);

    uint32_t new_fitness = evaluateFitness();

    if (new_fitness > fitness){
        staleTime = 0.f;
        fitness = new_fitness;
    }

    staleTime += elapsed;
    if (staleTime > 5000.f){
        dead = true;
    }
}

CarAgent::CarAgent(uint16_t populationID, const shared_ptr<Car> &car, const shared_ptr<ONFSTrack> &training_track) : track(training_track) {
    this->populationID = populationID;
    this->car = std::make_shared<Car>(car->all_models, car->tag, car->name);
    this->car->colour = glm::vec3(Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f));
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


