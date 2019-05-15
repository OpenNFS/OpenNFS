#pragma once

#include <vector>
#include "stdint.h"

#include "CarAgent.h"
#include "../Loaders/TrackLoader.h"
#include "../Loaders/CarLoader.h"
#include "../Physics/PhysicsEngine.h"
#include "../Util/Utils.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/RaceNetRenderer.h"
#include "../RaceNet/RaceNet.h"
#include "../RaceNet/RaceNEAT.h"

static const float stepTime = 1 / 60.f;

class TrainingGround {
public:
    explicit TrainingGround(uint16_t nGenerations, uint32_t nTicks, std::shared_ptr<ONFSTrack> &training_track, std::shared_ptr<Car> &training_car, std::shared_ptr<Logger> &logger, GLFWwindow *gl_window);
private:
    void TrainAgents(uint16_t nGenerations, uint32_t nTicks); // Train the agents, returning agent fitness data
    GLFWwindow *window;
    std::shared_ptr<ONFSTrack> training_track;
    std::shared_ptr<Car> training_car;
    std::vector<CarAgent> carAgents;
    RaceNetRenderer raceNetRenderer;
    /*------- BULLET --------*/
    PhysicsEngine physicsEngine;
};
