//
// Created by Amrik on 28/10/2018.
//

#pragma once

#include <vector>
#include "stdint.h"

#include "../Loaders/trk_loader.h"
#include "../Loaders/nfs3_loader.h"
#include "../Physics/Physics.h"

class MutationController {
public:
    explicit MutationController(uint16_t populationSize, uint16_t nGenerations, shared_ptr<ONFSTrack> training_track);
    float EvaluateFitness(shared_ptr<Car> car_agent);
private:
    shared_ptr<ONFSTrack> training_track;
    std::vector<shared_ptr<Car>> car_agents;
    /*------- BULLET --------*/
    Physics physicsEngine;
};
