#pragma once

#include "../Physics/PhysicsEngine.h"
#include "../Scene/Track.h"
#include "../RaceNet/Agents/PlayerAgent.h"
#include "../RaceNet/Agents/RacerAgent.h"

#include <unordered_set>
#include <vector>

namespace OpenNFS {
    class RacerManager {
    public:
        explicit RacerManager(const Track &track);
        void Init(const std::shared_ptr<PlayerAgent> &playerAgent, PhysicsEngine &physicsEngine);
        void Simulate() const;
        std::vector<uint32_t> GetRacerResidentTrackblocks() const;

        std::vector<std::shared_ptr<CarAgent>> racers;

    private:
        void _InitialisePlayerVehicle(const std::shared_ptr<PlayerAgent> &playerAgent, PhysicsEngine &physicsEngine);
        void _SpawnRacers(PhysicsEngine &physicsEngine);

        const Track& m_currentTrack;
    };
} // namespace OpenNFS
