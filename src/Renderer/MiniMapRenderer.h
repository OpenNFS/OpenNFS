#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "../RaceNet/Agents/CarAgent.h"
#include "../Scene/Track.h"
#include "../Shaders/MiniMapShader.h"

namespace OpenNFS {
    class MiniMapRenderer {
      public:
        explicit MiniMapRenderer();
        void Render(Track const &track, std::vector<std::shared_ptr<CarAgent>> const &racers);

      private:
        static constexpr float kVehicleScale {50.f};
        glm::mat4 projectionMatrix;
        MiniMapShader miniMapShader;
        // TODO: Offset from resolution
        float minX = -1000.f, maxX = 200.f, minY = 200, maxY = -800;
    };
}; // namespace OpenNFS
