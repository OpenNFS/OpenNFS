#pragma once

#include "../Camera/BaseCamera.h"
#include "../Physics/Car.h"
#include "Shaders/CarShader.h"

namespace OpenNFS {
    class CarRenderer {
      public:
        explicit CarRenderer() = default;
        ~CarRenderer();
        void Render(std::shared_ptr<Car> const &car, BaseCamera const &camera);

      private:
        // Create and compile our GLSL programs from the shaders
        CarShader m_carShader;
    };
} // namespace OpenNFS
