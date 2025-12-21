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
        CarAgent(AgentType agentType, std::shared_ptr<Car> const &car, Track const &track);
        virtual ~CarAgent() = default;
        void ResetToIndexInTrackblock(int trackBlockIndex, int posIndex, float offset) const;
        void ResetToVroad(int vroadIndex, float offset) const;
        virtual void Simulate() = 0;

        std::string name;
        std::shared_ptr<Car> vehicle;
        uint32_t m_nearestTrackblockID = 0;

      protected:
        void _UpdateNearestTrackblock();
        void _UpdateNearestVroad();

        Track const &m_track;
        AgentType m_agentType;
        uint32_t m_nearestVroadID = 0;
    };
} // namespace OpenNFS
