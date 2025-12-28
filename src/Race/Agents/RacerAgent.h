#pragma once

#include "CarAgent.h"

namespace OpenNFS {
    enum RacerAIMode {
        NeuralNet,
        Primitive
    };

    enum class RacerState {
        TRACK_FOLLOWING,  // Normal racing, following the track
        STUCK_REVERSING,   // Detected as stuck, reversing to get unstuck
        RESETTING,
    };

    class RacerAgent final : public CarAgent {
      public:
        RacerAgent(uint16_t racerID, std::shared_ptr<Car> const &car, std::shared_ptr<Track> const &raceTrack);
        void Simulate() override;

        uint32_t m_futureVroadID = 0;
        glm::vec3 targetVroadPosition;

      private:
        void _UseNeuralNetAI() const;
        void _UsePrimitiveAI();
        [[nodiscard]] uint32_t _CarSpeedToLookahead(float carSpeed) const;
        [[nodiscard]] float _CarSpeedToSteeringDamper(float carSpeed) const;

        // Configuration constants
        static constexpr uint32_t kBlockTickLimit{300};
        static constexpr float kSteeringDamper{.6f};
        static constexpr float kMinSpeed{20.f};
        static constexpr float kStuckSpeedThreshold{5.f};      // Speed below this is considered stuck (km/h)
        static constexpr uint32_t kStuckTicksThreshold{200};    // Ticks at low speed before considered stuck
        static constexpr uint32_t kReverseTicksLimit{60};      // How long to reverse when stuck
        static constexpr uint32_t kMaxReverseAttempts{3};      // Max reverse attempts before reset

        // AI state
        RacerAIMode m_mode = Primitive;
        RacerState m_state = RacerState::TRACK_FOLLOWING;
        RacerState m_nextState = RacerState::TRACK_FOLLOWING;;

        // Identity and tracking
        uint32_t m_racerID = 0;
        uint32_t m_lastTrackBlockID = 0;
        float m_steeringAngle = 0.f;

        // State counters
        uint32_t m_ticksInBlock = 0;          // Ticks spent in current track block
        uint32_t m_ticksGoingBackwards = 0;   // Consecutive ticks going backwards
        uint32_t m_ticksStuck = 0;            // Consecutive ticks at low speed
        uint32_t m_ticksReversing = 0;        // Ticks spent in STUCK_REVERSING state
        uint32_t m_reverseAttempts = 0;       // Number of reverse attempts made
    };
} // namespace OpenNFS
