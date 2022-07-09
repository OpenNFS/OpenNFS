#pragma once

#include "BaseCamera.h"
#include "../Renderer/HermiteCurve.h"

class HermiteCamera : public BaseCamera {
public:
    explicit HermiteCamera() = default;
    HermiteCamera(const HermiteCurve &trackCenterSpline, const std::shared_ptr<GLFWwindow> &window);
    void UseSpline(float elapsedTime);

private:
    HermiteCurve m_trackCameraRail;
    int m_loopTime = 0;
};
