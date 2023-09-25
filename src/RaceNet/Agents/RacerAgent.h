#pragma once

#include "CarAgent.h"

namespace OpenNFS {
    enum RacerAIMode { NeuralNet, Primitive, FollowTrack };

    class RacerAgent : public CarAgent {
    public:
        RacerAgent(uint16_t racerID, const std::shared_ptr<Car> &car, const std::shared_ptr<Track> &raceTrack);
        void Simulate() override;

    private:
        void _UseNeuralNetAI();
        void _UsePrimitiveAI();
        void _FollowTrack();

        RacerAIMode m_mode    = RacerAIMode::FollowTrack;
        uint32_t m_ticksAlive = 0;
    };
} // namespace OpenNFS
