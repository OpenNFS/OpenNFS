#pragma once

#include <Entities/BaseLight.h>

#include "../Camera/BaseCamera.h"
#include "../Config.h"
#include "../Race/Agents/CarAgent.h"
#include "Entities/BaseLight.h"
#include "Shaders/BillboardShader.h"
#include "Shaders/TrackShader.h"

namespace OpenNFS {
    class TrackRenderer {
      public:
        explicit TrackRenderer() = default;
        ~TrackRenderer();
        void Render(std::vector<std::shared_ptr<CarAgent>> const &racers,
                    BaseCamera const &camera,
                    GLuint trackTextureArrayID,
                    std::vector<std::shared_ptr<Entity>> const &visibleEntities,
                    std::vector<LibOpenNFS::BaseLight const *> const &lights,
                    ParamData const &userParams,
                    GLuint depthTextureID,
                    float ambientFactor);
        void RenderLights(BaseCamera const &camera, std::vector<LibOpenNFS::BaseLight const *> const &lights);

      private:
        // Create and compile our GLSL programs from the shaders
        TrackShader m_trackShader;
        BillboardShader m_billboardShader;
    };
} // namespace OpenNFS
