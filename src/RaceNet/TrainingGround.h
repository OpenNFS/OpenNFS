//
// Created by Amrik on 28/10/2018.
//

#pragma once

#include "RaceNet.h"
#include "../Physics/Physics.h"

class TrainingGround {
private:
    uint16_t populationID;
    shared_ptr<ONFSTrack> track;
    shared_ptr<Car> car;
    RaceNet carNetUnderTest;

    /*------- BULLET --------*/
    Physics physicsEngine;
public:
    explicit TrainingGround(uint16_t populationID, shared_ptr<ONFSTrack> track, shared_ptr<Car> car);
    void Step(float stepTime);
    float EvaluateFitness();
};

