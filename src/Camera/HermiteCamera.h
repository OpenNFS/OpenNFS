#pragma once

#include "BaseCamera.h"
#include "../Renderer/HermiteCurve.h"

namespace OpenNFS {
    class HermiteCamera : public BaseCamera {
    public:
        HermiteCamera(const HermiteCurve &trackCenterSpline, const InputManager &inputManager);

        void UseSpline(float elapsedTime);

    private:
        HermiteCurve m_trackCameraRail;
        int m_loopTime = 0;
    };
}
