#pragma once

#include "CarAgent.h"
#include <string>

namespace OpenNFS {
    struct RacerData {
        std::string name;
        std::string favouriteVehicle;
    };

    enum RacerAIMode {
        NeuralNet,
        Primitive
    };

    enum class RacerState {
        TRACK_FOLLOWING, // Normal racing, following the track
        STUCK_REVERSING, // Detected as stuck, reversing to get unstuck
        RESETTING,
    };

    class RacerAgent final : public CarAgent {
      public:
        RacerAgent(RacerData const &racerData, std::shared_ptr<Car> const &car, std::shared_ptr<Track> const &raceTrack);
        void Simulate() override;
        RacerState State() const;

        uint32_t m_futureVroadID = 0;
        glm::vec3 targetVroadPosition;

      private:
        void _UseNeuralNetAI() const;
        void _UsePrimitiveAI();
        [[nodiscard]] uint32_t _CarSpeedToLookahead(float carSpeed) const;
        [[nodiscard]] float _CarSpeedToSteeringDamper(float carSpeed) const;

        // Configuration constants
        static constexpr uint32_t kBlockTickLimit{200};
        static constexpr float kSteeringDamper{.6f};
        static constexpr float kMinSpeed{20.f};
        static constexpr uint32_t kStuckTicksThreshold{60}; // Ticks at low speed before considered stuck
        static constexpr uint32_t kReverseTicksLimit{60};   // How long to reverse when stuck
        static constexpr uint32_t kMaxReverseAttempts{4};   // Max reverse attempts before reset

        // AI state
        RacerAIMode m_mode = Primitive;
        RacerState m_state = RacerState::TRACK_FOLLOWING;
        RacerState m_nextState = RacerState::TRACK_FOLLOWING;

        // Identity and tracking
        uint32_t m_racerID = 0;
        uint32_t m_lastTrackBlockID = 0;
        float m_steeringAngle = 0.f;

        // State counters
        uint32_t m_ticksInBlock = 0;                         // Ticks spent in current track block
        uint32_t m_ticksStuck = 0;                           // Consecutive ticks at low speed
        uint32_t m_ticksReversing = 0;                       // Ticks spent in STUCK_REVERSING state
        uint32_t m_reverseAttempts = 0;                      // Number of reverse attempts made
        uint32_t m_currentReverseLimit = kReverseTicksLimit; // Dynamic reverse duration based on obstacle proximity
    };
} // namespace OpenNFS
