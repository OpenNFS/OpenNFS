#include "RacerManager.h"

RacerManager::RacerManager(PlayerAgent &playerAgent, std::shared_ptr<ONFSTrack> track, PhysicsEngine &physicsEngine) : m_currentTrack(track)
{
    // Add the player vehicle into the list of racers
    racers.emplace_back(playerAgent);
    this->_SpawnRacers(Config::get().nRacers, physicsEngine);
}

void RacerManager::Simulate()
{
    for (auto &racer : racers)
    {
        racer.Simulate();
    }
}

// Return a list of trackblocks on which racers directly reside
std::vector<uint32_t> RacerManager::GetRacerActiveTrackblocks()
{
    std::unordered_set<uint32_t> activeTrackblockIDs;

    for (auto &racer : racers)
    {
        activeTrackblockIDs.insert(racer.nearestTrackblockID);
    }

    return std::vector<uint32_t>(activeTrackblockIDs.begin(), activeTrackblockIDs.end());
}

void RacerManager::_SpawnRacers(uint8_t nRacers, PhysicsEngine &physicsEngine)
{
    std::shared_ptr<Car> racerVehicle = CarLoader::LoadCar(NFSVer::NFS_3, "f355");
    float racerSpawnOffset = -0.25f;
    for (uint8_t racerIdx = 0; racerIdx < nRacers; ++racerIdx)
    {
        RacerAgent racer(racerIdx % 23, BEST_NETWORK_PATH, racerVehicle, m_currentTrack);
        physicsEngine.RegisterVehicle(racer.vehicle);
        racer.ResetToIndexInTrackblock(0, racerIdx + 1, racerSpawnOffset);
        racerSpawnOffset = -racerSpawnOffset;
        racers.emplace_back(racer);
    }
}
