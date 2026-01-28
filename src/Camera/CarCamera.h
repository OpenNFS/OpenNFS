#pragma once

#include "../Physics/Car.h"

#include "BaseCamera.h"

namespace OpenNFS {
    class CarCamera : public BaseCamera {
      public:
        explicit CarCamera(InputManager const &inputManager);
        void FollowCar(std::shared_ptr<Car> const &targetCar, float deltaTime = 0.016f);
        void EnableCarouselMode(float initialAngularVelocity = 20.f);
        void DisableCarouselMode();

      private:
        void _CalculateZoom();
        void _CalculatePitch();
        void _CalculateAngleAroundCar(float deltaTime);
        [[nodiscard]] float _CalculateHorizontalDistance() const;
        [[nodiscard]] float _CalculateVerticalDistance() const;
        void _CalculateCameraPosition(std::shared_ptr<Car> const &target_car, float horizDistance, float vertDistance);

        float m_distanceFromCar = 10.f;
        float m_angleAroundCar = 0.f;
        float m_pitch = 10.f;
        float m_yaw = 0.f;

        // Carousel variables
        bool m_carouselMode = false;
        float m_angularVelocity = 0.f;
        float m_initialAngularVelocity = 0.f;
        float m_velocityDamping = 0.99f;
        float const kCarouselMinZoom{5.0f};
        float const kCarouselMaxZoom{20.f};
    };
} // namespace OpenNFS