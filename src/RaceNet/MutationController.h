//
// Created by Amrik on 28/10/2018.
//

#pragma once

#include <vector>
#include "stdint.h"

#include "TrainingGround.h"
#include "../Loaders/nfs3_loader.h"

class MutationController {
public:
    explicit MutationController(uint16_t populationSize, uint16_t nGenerations, shared_ptr<ONFSTrack> training_track);
private:
    std::vector<shared_ptr<TrainingGround>> trainingGrounds;
};
