#include "RacerManager.h"

#include "../Loaders/CarLoader.h"

namespace OpenNFS {
    RacerManager::RacerManager(const Track &track) : m_currentTrack(track) {
    }

    void RacerManager::Init(const std::shared_ptr<PlayerAgent> &playerAgent, PhysicsManager &physicsEngine) {
        this->_InitialisePlayerVehicle(playerAgent, physicsEngine);
        this->_SpawnRacers(physicsEngine);
    }

    void RacerManager::Simulate() const {
        for (auto &racer: racers) {
            racer->Simulate();
        }
    }

    // Return a list of trackblocks on which racers directly reside
    std::vector<uint32_t> RacerManager::GetRacerResidentTrackblocks() const {
        std::unordered_set<uint32_t> activeTrackblockIDs;

        for (auto &racer: racers) {
            activeTrackblockIDs.insert(racer->m_nearestTrackblockID);
        }

        return std::vector(activeTrackblockIDs.begin(), activeTrackblockIDs.end());
    }

    // Reset player character to start and add the player vehicle into the list of racers
    void RacerManager::_InitialisePlayerVehicle(const std::shared_ptr<PlayerAgent> &playerAgent, PhysicsManager &physicsEngine) {
        physicsEngine.RegisterVehicle(playerAgent->vehicle);
        playerAgent->ResetToIndexInTrackblock(0, 0, 0.f);
        racers.emplace_back(playerAgent);
    }

    // Spawn racers onto the track along Vroad positions at alternating offsets
    void RacerManager::_SpawnRacers(PhysicsManager &physicsEngine) {
        if (Config::get().nRacers == 0)
            return;

        std::shared_ptr<Car> racerVehicle {CarLoader::LoadCar(NFSVersion::NFS_3, "f355")};
        float racerSpawnOffset {-0.25f};
        for (uint32_t racerIdx = 0; racerIdx < Config::get().nRacers; ++racerIdx) {
            auto racer {std::make_shared<RacerAgent>(racerIdx % 23, racerVehicle, m_currentTrack)};
            physicsEngine.RegisterVehicle(racer->vehicle);
            racer->ResetToIndexInTrackblock(0, static_cast<int>(racerIdx + 1), racerSpawnOffset);
            racerSpawnOffset = -racerSpawnOffset;
            racers.emplace_back(racer);
        }
    }
} // namespace OpenNFS
