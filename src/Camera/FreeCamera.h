#pragma once

#include "Camera.h"

class FreeCamera : public Camera {
public:
    explicit FreeCamera() = default;
    FreeCamera(glm::vec3 initialPosition, GLFWwindow *window);
    void ComputeMatricesFromInputs(float deltaTime);
};

