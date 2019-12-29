#pragma once

#include "../Shaders/CarShader.h"
#include "../Camera/Camera.h"

class CarRenderer {
public:
    explicit CarRenderer() = default;
    ~CarRenderer();
    void Render(const shared_ptr<Car> &car, const std::shared_ptr<Camera> &camera, const std::vector<Light> &contributingLights);
private:
    // Create and compile our GLSL programs from the shaders
    CarShader m_carShader;
};
