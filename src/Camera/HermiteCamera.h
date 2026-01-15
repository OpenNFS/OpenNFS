#pragma once

#include "../Renderer/HermiteCurve.h"
#include "BaseCamera.h"

namespace OpenNFS {
    class HermiteCamera : public BaseCamera {
      public:
        HermiteCamera(HermiteCurve trackCenterSpline, InputManager const &inputManager);
        void UseSpline(float elapsedTime);

      private:
        HermiteCurve m_trackCameraRail;
        int m_loopTime = 0;
    };
} // namespace OpenNFS
