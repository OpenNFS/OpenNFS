#pragma once

#include "../../Scene/Track.h"

namespace OpenNFS {
    class Car;

    enum AgentType : uint8_t {
        TRAINING = 0,
        RACING,
        PLAYER,
    };

    class CarAgent {
    public:
        virtual ~CarAgent() = default;

        CarAgent(AgentType agentType, const std::shared_ptr<Car> &car, const Track &track);
        void ResetToIndexInTrackblock(int trackBlockIndex, int posIndex, float offset) const;
        void ResetToVroad(int vroadIndex, float offset) const;
        virtual void Simulate() = 0;

        std::string name;
        std::shared_ptr<Car> vehicle;
        uint32_t m_nearestTrackblockID = 0;

    protected:
        void _UpdateNearestTrackblock();
        void _UpdateNearestVroad();

        const Track& m_track;
        AgentType m_agentType;
        uint32_t m_nearestVroadID = 0;
    };
} // namespace OpenNFS
