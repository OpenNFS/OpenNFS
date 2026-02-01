#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "../Scene/Track.h"
#include "Shaders/MiniMapShader.h"

namespace OpenNFS {
    class CarAgent;
    class MiniMapRenderer {
      public:
        explicit MiniMapRenderer() = default;
        ~MiniMapRenderer();
        void Render(std::shared_ptr<Track> const &track, std::vector<std::shared_ptr<CarAgent>> const &racers, int windowWidth,
                    int windowHeight);

      private:
        void _Rescale();

        static constexpr float kVehicleScale{40.f};
        static constexpr float kMinimapFraction{0.25f}; // 25% of smaller window dimension
        static constexpr int kMinimapPadding{100};      // Pixels from screen edge

        glm::mat4 projectionMatrix{};
        MiniMapShader m_miniMapShader;
        float left{0.f};
        float right{0.f};
        float bottom{0.f};
        float top{0.f};
    };
} // namespace OpenNFS
