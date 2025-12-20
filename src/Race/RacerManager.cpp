#include "RacerManager.h"

#include "../Loaders/CarLoader.h"

namespace OpenNFS {
    RacerManager::RacerManager(std::shared_ptr<PlayerAgent> const &playerAgent, PhysicsManager &physicsManager, Track const &track)
        : m_currentTrack(track) {
        this->_InitialisePlayerVehicle(playerAgent, physicsManager);
        this->_SpawnRacers(physicsManager);
    }

    void RacerManager::Simulate() const {
        for (auto &racer : racers) {
            racer->Simulate();
        }
    }

    // Return a list of trackblocks on which racers directly reside
    std::vector<uint32_t> RacerManager::GetRacerResidentTrackblocks() const {
        std::unordered_set<uint32_t> activeTrackblockIDs;

        for (auto &racer : racers) {
            activeTrackblockIDs.insert(racer->m_nearestTrackblockID);
        }

        return {activeTrackblockIDs.begin(), activeTrackblockIDs.end()};
    }

    // Reset player character to start and add the player vehicle into the list of racers
    void RacerManager::_InitialisePlayerVehicle(std::shared_ptr<PlayerAgent> const &playerAgent, PhysicsManager &physicsManager) {
        physicsManager.RegisterVehicle(playerAgent->vehicle);
        playerAgent->ResetToIndexInTrackblock(0, 0, 0.f);
        racers.emplace_back(playerAgent);
    }

    // Spawn racers onto the track along Vroad positions at alternating offsets
    void RacerManager::_SpawnRacers(PhysicsManager &physicsManager) {
        if (Config::get().nRacers == 0)
            return;

        std::shared_ptr<Car> racerVehicle{CarLoader::LoadCar(NFSVersion::NFS_3, "f355")};
        float racerSpawnOffset{-0.25f};
        for (uint32_t racerIdx = 0; racerIdx < Config::get().nRacers; ++racerIdx) {
            auto racer{std::make_shared<RacerAgent>(racerIdx % 23, racerVehicle, m_currentTrack)};
            physicsManager.RegisterVehicle(racer->vehicle);
            racer->ResetToIndexInTrackblock(0, static_cast<int>(racerIdx + 1), racerSpawnOffset);
            racerSpawnOffset = -racerSpawnOffset;
            racers.emplace_back(racer);
        }
    }
} // namespace OpenNFS
