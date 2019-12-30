#pragma once

#include "BaseCamera.h"

class FreeCamera : public BaseCamera {
public:
    explicit FreeCamera() = default;
    FreeCamera(GLFWwindow *window, glm::vec3 initialPosition);
    void ComputeMatricesFromInputs(float deltaTime);

};

