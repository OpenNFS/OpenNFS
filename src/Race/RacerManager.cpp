#include "RacerManager.h"

namespace OpenNFS {
    RacerManager::RacerManager(const std::shared_ptr<PlayerAgent> &playerAgent, const std::shared_ptr<Track> &track, PhysicsEngine &physicsEngine) : m_currentTrack(track) {
        this->_InitialisePlayerVehicle(playerAgent, physicsEngine);
        this->_SpawnRacers(physicsEngine);
    }

    void RacerManager::Simulate() {
        for (auto &racer : racers) {
            racer->Simulate();
        }
    }

    // Return a list of trackblocks on which racers directly reside
    std::vector<uint32_t> RacerManager::GetRacerResidentTrackblocks() {
        std::unordered_set<uint32_t> activeTrackblockIDs;

        for (auto &racer : racers) {
            activeTrackblockIDs.insert(racer->nearestTrackblockID);
        }

        return std::vector<uint32_t>(activeTrackblockIDs.begin(), activeTrackblockIDs.end());
    }

    // Reset player character to start and add the player vehicle into the list of racers
    void RacerManager::_InitialisePlayerVehicle(const std::shared_ptr<PlayerAgent> &playerAgent, PhysicsEngine &physicsEngine) {
        physicsEngine.RegisterVehicle(playerAgent->vehicle);
        playerAgent->ResetToIndexInTrackblock(0, 0, 0.f);
        racers.emplace_back(playerAgent);
    }

    // Spawn racers onto the track along Vroad positions at alternating offsets
    void RacerManager::_SpawnRacers(PhysicsEngine &physicsEngine) {
        if (Config::get().nRacers == 0)
            return;

        std::shared_ptr<Car> racerVehicle = CarLoader::LoadCar(NFSVersion::NFS_3, "f355");
        float racerSpawnOffset            = -0.25f;
        for (uint8_t racerIdx = 0; racerIdx < Config::get().nRacers; ++racerIdx) {
            std::shared_ptr<RacerAgent> racer = std::make_shared<RacerAgent>(racerIdx % 23, racerVehicle, m_currentTrack);
            physicsEngine.RegisterVehicle(racer->vehicle);
            racer->ResetToIndexInTrackblock(0, racerIdx + 1, racerSpawnOffset);
            racerSpawnOffset = -racerSpawnOffset;
            racers.emplace_back(racer);
        }
    }
} // namespace OpenNFS
