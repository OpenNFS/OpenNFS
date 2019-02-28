//
// Created by Amrik on 28/10/2018.
//

#pragma once

#include <vector>
#include "stdint.h"

#include "CarAgent.h"
#include "../Loaders/trk_loader.h"
#include "../Loaders/car_loader.h"
#include "../Physics/Physics.h"
#include "../Util/Utils.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/RaceNetRenderer.h"
#include "../RaceNet/RaceNet.h"
#include "../RaceNet/RaceNEAT.h"

static const float stepTime = 1 / 60.f;

class TrainingGround {
public:
    explicit TrainingGround(uint16_t populationSize, uint16_t nGenerations, uint32_t nTicks, shared_ptr<ONFSTrack> &training_track, shared_ptr<Car> &training_car, std::shared_ptr<Logger> &logger, GLFWwindow *gl_window);
private:
    void TrainAgents(uint16_t nGenerations, uint32_t nTicks); // Train the agents, returning agent fitness data
    GLFWwindow *window;
    shared_ptr<ONFSTrack> training_track;
    shared_ptr<Car> training_car;
    std::vector<CarAgent> car_agents;
    RaceNetRenderer raceNetRenderer;
    /*------- BULLET --------*/
    Physics physicsEngine;
};
