#pragma once

#include "RaceNet.h"
#include "RaceNEAT.h"
#include "../Loaders/CarLoader.h"
#include "../Loaders/TrackLoader.h"

static const uint32_t STALE_TICK_COUNT = 70;

class CarAgent
{
public:
    CarAgent(uint16_t populationID, std::shared_ptr<Car> trainingCar, std::shared_ptr<ONFSTrack> trainingTrack); // Training
    CarAgent(uint16_t racerID, const std::string &networkPath, std::shared_ptr<Car> car, std::shared_ptr<ONFSTrack> trainingTrack); // Racing
    CarAgent() = default; // std::vector raw impl

    void resetToVroad(int trackBlockIndex, int posIndex, float offset, const std::shared_ptr<ONFSTrack> &track);
    void resetToVroad(int vroadIndex, float offset, const std::shared_ptr<ONFSTrack> &track);
    int getClosestVroad(const std::shared_ptr<Car> &car, const std::shared_ptr<ONFSTrack> &track);


    void reset(); // Wrapper to reset to start of training track
    bool isWinner();
    void simulate();

    std::shared_ptr<Car> car;
    RaceNet raceNet;
    std::string name;
    uint16_t populationID = UINT16_MAX;
    bool training = false;
    int fitness = 0;
    int insideVroadCount = 0;
    float averageSpeed = 0.f;
    bool dead = false;
    bool droveBack = false;

private:
    int _EvaluateFitness(int vroadPosition);
    std::shared_ptr<ONFSTrack> track;
    int tickCount = 0;
};

