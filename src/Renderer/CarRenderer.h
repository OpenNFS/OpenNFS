#pragma once

#include "../Shaders/CarShader.h"
#include "../Camera/BaseCamera.h"

class CarRenderer {
public:
    explicit CarRenderer() = default;
    ~CarRenderer();
    void Render(const shared_ptr<Car> &car, const BaseCamera &camera, const std::vector<std::shared_ptr<BaseLight>> &lights);

private:
    // Create and compile our GLSL programs from the shaders
    CarShader m_carShader;
};
