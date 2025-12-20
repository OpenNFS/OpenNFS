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
        explicit RacerManager(std::shared_ptr<PlayerAgent> const &playerAgent, PhysicsManager &physicsManager, Track const &track);
        void Simulate() const;
        [[nodiscard]] std::vector<uint32_t> GetRacerResidentTrackblocks() const;

        std::vector<std::shared_ptr<CarAgent>> racers;

      private:
        void _InitialisePlayerVehicle(std::shared_ptr<PlayerAgent> const &playerAgent, PhysicsManager &physicsManager);
        void _SpawnRacers(PhysicsManager &physicsManager);

        Track const &m_currentTrack;
    };
} // namespace OpenNFS
