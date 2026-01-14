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
        void Render(std::shared_ptr<Track> const &track, std::vector<std::shared_ptr<CarAgent>> const &racers);

      private:
        void _Rescale();

        static constexpr float kVehicleScale{50.f};
        static constexpr float kMiniMapWidth{1000.f};
        static constexpr float kMiniMapHeight{1000.f};
        static constexpr float kMiniMapOffsetX{1000.f};
        static constexpr float kMiniMapOffsetY{1000.f};

        glm::mat4 projectionMatrix{};
        MiniMapShader m_miniMapShader;
        float left{0.f};
        float right{0.f};
        float bottom{0.f};
        float top{0.f};
    };
} // namespace OpenNFS
