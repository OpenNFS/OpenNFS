#pragma once

#include "CarAgent.h"

class RacerAgent : public CarAgent
{
public:
    RacerAgent(uint16_t racerID, const std::string &networkPath, const std::shared_ptr<Car> &car, const std::shared_ptr<Track> &raceTrack);
    void Simulate() override;
};