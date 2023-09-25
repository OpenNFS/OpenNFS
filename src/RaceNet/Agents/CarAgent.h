#pragma once

#include "../../Loaders/CarLoader.h"
#include "../../Scene/Track.h"

namespace OpenNFS {
    enum AgentType : uint8_t {
        TRAINING = 0,
        RACING,
        PLAYER,
    };

    class CarAgent {
    public:
        CarAgent(AgentType agentType, const std::shared_ptr<Car> &car, const std::shared_ptr<OpenNFS::Track> &track);
        void ResetToIndexInTrackblock(int trackBlockIndex, int posIndex, float offset);
        void ResetToVroad(int vroadIndex, float offset);
        virtual void Simulate() = 0;

        std::string name;
        std::shared_ptr<Car> vehicle;
        uint32_t nearestTrackblockID = 0;

    protected:
        void _UpdateNearestTrackblock();
        void _UpdateNearestVroad();

        std::shared_ptr<Track> m_track;
        AgentType m_agentType;
        uint32_t m_nearestVroadID = 0;
    };
} // namespace OpenNFS
