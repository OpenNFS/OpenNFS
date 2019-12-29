#pragma once

#include "../Physics/PhysicsEngine.h"
#include "../Loaders/TrackLoader.h"
#include "../RaceNet/CarAgent.h"
#include <vector>

class RacerManager
{
public:
    explicit RacerManager() = default;
    RacerManager(std::shared_ptr<ONFSTrack> track, PhysicsEngine &physicsEngine);
    void Simulate();
private:
    void _SpawnRacers(uint8_t nRacers, PhysicsEngine &physicsEngine);

    std::vector<CarAgent> m_racers;
    std::shared_ptr<ONFSTrack> m_currentTrack;
};
