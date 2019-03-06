//
// Created by amrik on 28/02/19.
//

#pragma once

#include "RaceNet.h"
#include "RaceNEAT.h"
#include "../Loaders/car_loader.h"
#include "../Loaders/trk_loader.h"

static const uint32_t STALE_TICK_COUNT = 70;

class CarAgent {
private:
    std::shared_ptr<ONFSTrack> track;
    float tickCount = 0;
public:
    std::shared_ptr<Car> car;
    RaceNet raceNet;
    std::string name;
    uint16_t populationID = UINT16_MAX;
    bool training = false;
    int fitness = 0;
    bool dead = false;
    bool droveBack = false;

    CarAgent(uint16_t populationID, const shared_ptr<Car> &trainingCar, const shared_ptr<ONFSTrack> &trainingTrack); // Training
    CarAgent(const std::string &racerName, const std::string &networkPath, const shared_ptr<Car> &car, const shared_ptr<ONFSTrack> &trainingTrack); // Racing
    CarAgent() = default; // std::vector raw impl

    static void resetToVroad(int trackBlockIndex, int posIndex, float offset, std::shared_ptr<ONFSTrack> &track, std::shared_ptr<Car> &car);
    static void resetToVroad(int vroadIndex, float offset, std::shared_ptr<ONFSTrack> &track, std::shared_ptr<Car> &car);

    static int getClosestVroad(const std::shared_ptr<Car> &car, const std::shared_ptr<ONFSTrack> &track);
    void reset(); // Wrapper to reset to start of training track
    bool isWinner();
    void simulate();
};

