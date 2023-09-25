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
        explicit RacerManager() = default;
        RacerManager(const std::shared_ptr<PlayerAgent> &playerAgent, const std::shared_ptr<Track> &track, PhysicsEngine &physicsEngine);
        void Simulate();
        std::vector<uint32_t> GetRacerResidentTrackblocks();

        std::vector<std::shared_ptr<CarAgent>> racers;

    private:
        void _InitialisePlayerVehicle(const std::shared_ptr<PlayerAgent> &playerAgent, PhysicsEngine &physicsEngine);
        void _SpawnRacers(PhysicsEngine &physicsEngine);

        std::shared_ptr<Track> m_currentTrack;
    };
} // namespace OpenNFS
