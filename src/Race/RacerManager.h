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
        explicit RacerManager(const Track &track);
        void Init(const std::shared_ptr<PlayerAgent> &playerAgent, PhysicsManager &physicsEngine);
        void Simulate() const;
        [[nodiscard]] std::vector<uint32_t> GetRacerResidentTrackblocks() const;

        std::vector<std::shared_ptr<CarAgent>> racers;

    private:
        void _InitialisePlayerVehicle(const std::shared_ptr<PlayerAgent> &playerAgent, PhysicsManager &physicsEngine);
        void _SpawnRacers(PhysicsManager &physicsEngine);

        const Track& m_currentTrack;
    };
} // namespace OpenNFS
