//
// Created by amrik on 28/02/19.
//

#pragma once

#include "RaceNet.h"
#include "RaceNEAT.h"
#include "../Loaders/car_loader.h"
#include "../Loaders/trk_loader.h"

static const uint32_t STALE_TICK_COUNT = 300;

typedef struct {
    float angle;
    float curLapTime;
    float distFromStartLine;
    float distRaced;
    float lastLapTime;
    float opponents[36];
    int racePos;
    float speedX;
    float speedY;
    float trackPos;
    float wheelSpinVel;
} SensorPack;

class CarAgent {
private:
    std::shared_ptr<ONFSTrack> track;
    float tickCount = 0;
public:
    std::shared_ptr<Car> car;
    RaceNet raceNet;
    std::string name;
    uint16_t populationID = UINT16_MAX;
    int fitness = 0;
    bool dead = false;

    CarAgent(uint16_t populationID, const shared_ptr<Car> &trainingCar, const shared_ptr<ONFSTrack> &trainingTrack); // Training
    CarAgent(const std::string &racerName, const std::string &networkPath, const shared_ptr<Car> &car); // Racing
    CarAgent() = default; // std::vector raw impl

    static void resetToVroad(int trackBlockIndex, int posIndex, float offset, std::shared_ptr<ONFSTrack> &track, std::shared_ptr<Car> &car);
    void reset(); // Wrapper to reset to start of training track
    bool isWinner();
    int evaluateFitness();
    void simulate();
};

