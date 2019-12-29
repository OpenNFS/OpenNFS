#include "RacerManager.h"

RacerManager::RacerManager(std::shared_ptr<ONFSTrack> track, PhysicsEngine &physicsEngine) : m_currentTrack(track)
{
    this->_SpawnRacers(Config::get().nRacers, physicsEngine);
}

void RacerManager::Simulate()
{
    for (auto &racer : m_racers)
    {
        racer.simulate();
    }
}

void RacerManager::_SpawnRacers(uint8_t nRacers, PhysicsEngine &physicsEngine)
{
    std::shared_ptr<Car> racerVehicle = CarLoader::LoadCar(NFSVer::NFS_3, "f355");
    float racerSpawnOffset = -0.25f;
    for (uint8_t racerIdx = 0; racerIdx < nRacers; ++racerIdx)
    {
        CarAgent racer(racerIdx % 23, BEST_NETWORK_PATH, racerVehicle, m_currentTrack);
        physicsEngine.RegisterVehicle(racer.car);

        //CarAgent::resetToVroad(0, racerIdx + 1, racerSpawnOffset, m_currentTrack, racer.car);
        racerSpawnOffset = -racerSpawnOffset;

        m_racers.emplace_back(racer);
    }
}
