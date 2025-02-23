#pragma once

#include "BaseCamera.h"

namespace OpenNFS {
    class FreeCamera : public BaseCamera {
    public:
        FreeCamera(const InputManager &inputManager, glm::vec3 initialPosition);

        void ComputeMatricesFromInputs(float deltaTime);
    };
}
