#pragma once

#include "../Physics/Car.h"

#include "BaseCamera.h"

namespace OpenNFS {
    class CarCamera : public BaseCamera {
    public:
        explicit CarCamera(const InputManager &inputManager);
        void FollowCar(const std::shared_ptr<Car> &targetCar);

    private:
        void _CalculateZoom();
        void _CalculatePitch();
        void _CalculateAngleAroundCar();
        [[nodiscard]] float _CalculateHorizontalDistance() const;
        [[nodiscard]] float _CalculateVerticalDistance() const;
        void _CalculateCameraPosition(const std::shared_ptr<Car> &target_car, float horizDistance, float vertDistance);

        float m_distanceFromCar = 0.7f;
        float m_angleAroundCar  = 0.f;
        float m_pitch           = 10.f;
        float m_yaw             = 0.f;
    };
} // namespace OpenNFS