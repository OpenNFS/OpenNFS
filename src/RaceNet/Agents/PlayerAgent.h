#pragma once

#include "CarAgent.h"

class PlayerAgent : public CarAgent {
public:
    PlayerAgent(std::shared_ptr<Car> car, std::shared_ptr<ONFSTrack> raceTrack);
    void Simulate();
};
