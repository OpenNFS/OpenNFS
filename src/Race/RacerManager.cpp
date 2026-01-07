#include "RacerManager.h"

#include <fstream>
#include <json/json.hpp>

#include "../Loaders/CarLoader.h"
#include "Agents/RacerAgent.h"

using json = nlohmann::json;

namespace OpenNFS {
    RacerManager::RacerManager(std::shared_ptr<PlayerAgent> const &playerAgent, PhysicsManager &physicsManager,
                               std::shared_ptr<Track> const &track)
        : m_currentTrack(track) {
        this->_LoadRacerData(RACER_NAME_PATH);
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

        // Fallback to default racer data if loading failed
        if (m_racerData.empty()) {
            LOG(WARNING) << "No racer data loaded, using default racer";
            m_racerData.push_back({"Unknown Racer", ""});
        }

        std::shared_ptr<Car> racerVehicle{CarLoader::LoadCar(NFSVersion::NFS_3, "f355")};
        float racerSpawnOffset{-0.25f};
        for (uint32_t racerIdx = 0; racerIdx < Config::get().nRacers; ++racerIdx) {
            uint32_t const racerDataIdx = racerIdx % m_racerData.size();
            auto racer{std::make_shared<RacerAgent>(m_racerData[racerDataIdx], racerVehicle, m_currentTrack)};
            physicsManager.RegisterVehicle(racer->vehicle);
            racer->ResetToIndexInTrackblock(0, static_cast<int>(racerIdx + 1), racerSpawnOffset);
            racerSpawnOffset = -racerSpawnOffset;
            racers.emplace_back(racer);
        }
    }

    void RacerManager::_LoadRacerData(std::string const &jsonPath) {
        std::ifstream file(jsonPath);
        if (!file.is_open()) {
            LOG(WARNING) << "Failed to open racer data file: " << jsonPath;
            return;
        }

        try {
            json racerJson;
            file >> racerJson;

            m_racerData.clear();
            for (auto const &racerEntry : racerJson["racers"]) {
                RacerData racer;
                racer.name = racerEntry.value("name", "Unknown");
                racer.favouriteVehicle = racerEntry.value("favouriteVehicle", "");
                m_racerData.push_back(racer);
            }

            LOG(INFO) << "Loaded " << m_racerData.size() << " racer profiles from " << jsonPath;
        } catch (json::exception const &e) {
            LOG(WARNING) << "Failed to parse racer data JSON: " << e.what();
        }
    }
} // namespace OpenNFS
