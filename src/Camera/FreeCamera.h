#pragma once

#include "Camera.h"

class FreeCamera : public Camera {
public:
    explicit FreeCamera() = default;
    FreeCamera(GLFWwindow *window, glm::vec3 initialPosition);
    void ComputeMatricesFromInputs(float deltaTime);

};

