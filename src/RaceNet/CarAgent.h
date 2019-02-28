//
// Created by amrik on 28/02/19.
//

#pragma once

#include <chrono>

#include "RaceNet.h"
#include "RaceNEAT.h"
#include "../Loaders/car_loader.h"
#include "../Loaders/trk_loader.h"


class Timer {
public:
    Timer() : m_beg(clock_::now()) {
    }
    void reset() {
        m_beg = clock_::now();
    }

    double elapsed() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                clock_::now() - m_beg).count();
    }

private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> m_beg;
};


class CarAgent {
private:
    Timer timer;
public:
    std::shared_ptr<Car> car;
    std::shared_ptr<ONFSTrack> track;
    RaceNet raceNet;
    uint16_t populationID = UINT16_MAX;
    uint32_t fitness = 0;
    double staleTime = 0.0;
    bool dead = false;

    CarAgent(uint16_t populationID, const shared_ptr<Car> &car, const shared_ptr<ONFSTrack> &training_track);
    CarAgent() = default;
    bool isWinner();
    uint32_t evaluateFitness();
    void simulate();
};

