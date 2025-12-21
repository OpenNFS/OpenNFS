#pragma once

#include "BaseCamera.h"

namespace OpenNFS {
    class FreeCamera : public BaseCamera {
      public:
        FreeCamera(InputManager const &inputManager, glm::vec3 initialPosition);

        void ComputeMatricesFromInputs(float deltaTime);
    };
} // namespace OpenNFS
