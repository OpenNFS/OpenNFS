//
// Created by Amrik Sadhra on 21/08/2018.
//

#pragma once


#include "../Shaders/CarShader.h"
#include "../Scene/Camera.h"

class CarRenderer {
public:
    explicit CarRenderer();
    ~CarRenderer();
    void render(shared_ptr<Car> &car, const Camera &mainCamera, const std::vector<Light> &contributingLights);
    // Create and compile our GLSL programs from the shaders
    CarShader carShader;
};
