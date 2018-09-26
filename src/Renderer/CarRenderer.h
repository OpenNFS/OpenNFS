//
// Created by Amrik Sadhra on 21/08/2018.
//

#pragma once


#include "../Shaders/CarShader.h"
#include "../Scene/Camera.h"

class CarRenderer {
public:
    explicit CarRenderer(const shared_ptr<Car> &activeCar);
    ~CarRenderer();
    void render(const Camera &mainCamera, const std::vector<Light> &contributingLights);
private:
    // Create and compile our GLSL programs from the shaders
    CarShader carShader;
    shared_ptr<Car> car;
};
