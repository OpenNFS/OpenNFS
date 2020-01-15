#pragma once

#include <vector>
#include "stdint.h"

#include "Agents/TrainingAgent.h"
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
    explicit TrainingGround(uint16_t nGenerations,
            uint32_t nTicks,
            const std::shared_ptr<ONFSTrack> &training_track,
            const std::shared_ptr<Car> &training_car,
            const std::shared_ptr<Logger> &logger,
            const std::shared_ptr<GLFWwindow> &window);
private:
    void TrainAgents(uint16_t nGenerations, uint32_t nTicks); // Train the agents, returning agent fitness data
    std::shared_ptr<GLFWwindow> m_window;
    std::shared_ptr<ONFSTrack> training_track;
    std::shared_ptr<Car> training_car;
    std::vector<TrainingAgent> trainingAgents;
    RaceNetRenderer raceNetRenderer;
    /*------- BULLET --------*/
    PhysicsEngine physicsEngine;
};
