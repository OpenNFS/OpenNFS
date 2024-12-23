#pragma once

#include "CarAgent.h"

namespace OpenNFS {
    enum RacerAIMode { NeuralNet, Primitive, FollowTrack };

    class RacerAgent final : public CarAgent {
    public:
        RacerAgent(uint16_t racerID, const std::shared_ptr<Car> &car, const Track &raceTrack);

        void Simulate() override;

    private:
        void _UseNeuralNetAI() const;
        void _UsePrimitiveAI() const;
        void _FollowTrack() const;

        static constexpr uint32_t kBlockTickLimit {200};
        RacerAIMode m_mode    = FollowTrack;
        uint32_t m_racerID = 0;
        uint32_t m_ticksInBlock = 0;
        uint32_t m_lastTrackBlockID = 0;
    };
} // namespace OpenNFS
