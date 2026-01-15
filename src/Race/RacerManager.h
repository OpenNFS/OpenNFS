#pragma once

#include "../Physics/PhysicsManager.h"
#include "../Race/Agents/PlayerAgent.h"
#include "../Race/Agents/RacerAgent.h"
#include "../Scene/Track.h"

#include <unordered_set>
#include <vector>

namespace OpenNFS {
    class RacerManager {
      public:
        explicit RacerManager(std::shared_ptr<PlayerAgent> const &playerAgent, PhysicsManager &physicsManager,
                              std::shared_ptr<Track> const &track);
        void Simulate() const;
        [[nodiscard]] std::vector<uint32_t> GetRacerResidentTrackblocks() const;

        std::vector<std::shared_ptr<CarAgent>> racers;

      private:
        void _InitialisePlayerVehicle(std::shared_ptr<PlayerAgent> const &playerAgent, PhysicsManager &physicsManager);
        void _SpawnRacers(PhysicsManager &physicsManager);
        void _LoadRacerData(std::string const &jsonPath);

        std::shared_ptr<Track> m_currentTrack;
        std::vector<RacerData> m_racerData;
    };
} // namespace OpenNFS
