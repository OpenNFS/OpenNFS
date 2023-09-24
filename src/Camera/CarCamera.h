#pragma once

#include "../Physics/Car.h"

#include "BaseCamera.h"

class CarCamera : public BaseCamera {
public:
    explicit CarCamera() = default;
    CarCamera(const std::shared_ptr<GLFWwindow> &window);
    void FollowCar(const std::shared_ptr<Car> &targetCar);

private:
    void _CalculateZoom();
    void _CalculatePitch();
    void _CalculateAngleAroundCar();
    float _CalculateHorizontalDistance();
    float _CalculateVerticalDistance();
    void _CalculateCameraPosition(const std::shared_ptr<Car> &target_car, float horizDistance, float vertDistance);

    float m_distanceFromCar = 0.7f;
    float m_angleAroundCar  = 0.f;
    float m_pitch           = 10.f;
    float m_yaw             = 0.f;
};