#pragma once

#include "../Physics/Car.h"
#include "../Shaders/CarShader.h"
#include "../Camera/BaseCamera.h"

namespace OpenNFS {
    class CarRenderer {
    public:
        explicit CarRenderer() = default;
        ~CarRenderer();
        void Render(const std::shared_ptr<Car> &car, const BaseCamera &camera, const std::vector<std::shared_ptr<LibOpenNFS::BaseLight>> &lights);

    private:
        // Create and compile our GLSL programs from the shaders
        CarShader m_carShader;
    };
} // namespace OpenNFS
