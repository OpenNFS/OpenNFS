#pragma once

#include "../Camera/BaseCamera.h"
#include "../Physics/Car.h"
#include "../Scene/GlobalLight.h"
#include "Shaders/CarShader.h"

namespace OpenNFS {
    class CarRenderer {
      public:
        explicit CarRenderer() = default;
        ~CarRenderer();
        void Render(std::shared_ptr<Car> const &car, BaseCamera const &camera, std::vector<BaseLight const *> const &trackLights,
                    GLuint shadowMapArrayID) const;

      private:
        // Create and compile our GLSL programs from the shaders
        CarShader m_carShader;
    };
} // namespace OpenNFS
