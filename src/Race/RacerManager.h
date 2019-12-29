#pragma once

#include "../Physics/PhysicsEngine.h"
#include "../Loaders/TrackLoader.h"
#include "../RaceNet/Agents/PlayerAgent.h"
#include "../RaceNet/Agents/RacerAgent.h"

#include <unordered_set>
#include <vector>

class RacerManager
{
public:
    explicit RacerManager() = default;
    RacerManager(PlayerAgent &playerAgent, std::shared_ptr<ONFSTrack> track, PhysicsEngine &physicsEngine);
    void Simulate();
    std::vector<uint32_t> GetRacerActiveTrackblocks();

    std::vector<CarAgent> racers;
private:
    void _SpawnRacers(uint8_t nRacers, PhysicsEngine &physicsEngine);

    std::shared_ptr<ONFSTrack> m_currentTrack;
};
