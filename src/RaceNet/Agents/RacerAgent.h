#pragma once

#include "CarAgent.h"

namespace OpenNFS {
    enum RacerAIMode { NeuralNet, Primitive };

    class RacerAgent final : public CarAgent {
    public:
        RacerAgent(uint16_t racerID, const std::shared_ptr<Car> &car, const Track &raceTrack);
        void Simulate() override;

    private:
        void _UseNeuralNetAI() const;
        void _UsePrimitiveAI();
        [[nodiscard]] uint32_t _CarSpeedToLookahead(float carSpeed) const;
        [[nodiscard]] float _CarSpeedToSteeringDamper(float carSpeed) const;

        static constexpr uint32_t kBlockTickLimit{300};
        static constexpr float kSteeringDamper{.6f};
        static constexpr float kMinSpeed{20.f};
        RacerAIMode m_mode    = Primitive;
        uint32_t m_racerID = 0;
        uint32_t m_ticksInBlock = 0;
        uint32_t m_ticksGoingBackwards = 0;
        uint32_t m_lastTrackBlockID = 0;
        float m_steeringAngle = 0.f;
    };
} // namespace OpenNFS
