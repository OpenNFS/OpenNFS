#pragma once

#include "CarAgent.h"

static const uint32_t STALE_TICK_COUNT = 70;

class TrainingAgent : public CarAgent
{
public:
    TrainingAgent(uint16_t populationID, std::shared_ptr<Car> trainingCar, std::shared_ptr<ONFSTrack> trainingTrack);
    void Simulate();
    void Reset(); // Wrapper to reset to start of training track
    bool IsWinner();

    int fitness = 0;
    bool isDead = false;
    int ticksInsideVroad = 0;
    float averageSpeed = 0.f;
    uint16_t populationID = UINT16_MAX;

private:
    int _EvaluateFitness(int vroadPosition);

    bool m_droveBack = false;
    int m_ticksSpentAlive = 0;
};

