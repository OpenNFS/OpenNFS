#pragma once

#include "Camera.h"
#include "../Renderer/HermiteCurve.h"

class HermiteCamera : public Camera {
public:
    explicit HermiteCamera() = default;
    HermiteCamera(const HermiteCurve &trackCenterSpline, GLFWwindow *window);
    void UseSpline(float elapsedTime);

private:
    HermiteCurve m_trackCameraRail;
    int m_loopTime = 0;
};

