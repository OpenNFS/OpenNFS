#pragma once

#include "../Shaders/CarShader.h"
#include "../Scene/Camera.h"

class CarRenderer {
public:
    explicit CarRenderer() = default;
    ~CarRenderer();
    void render(shared_ptr<Car> &car, const Camera &mainCamera, const std::vector<Light> &contributingLights);
    // Create and compile our GLSL programs from the shaders
    CarShader carShader;
};
