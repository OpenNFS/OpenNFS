#pragma once

#include <GL/glew.h>

#include "../Camera/BaseCamera.h"
#include "../Race/Agents/CarAgent.h"
#include "../Scene/GlobalLight.h"
#include "../Scene/Track.h"
#include "Shaders/DepthShader.h"

namespace OpenNFS {
    class ShadowMapRenderer {
      public:
        ShadowMapRenderer();
        ~ShadowMapRenderer();
        [[nodiscard]] GLuint GetTextureArrayID() const;
        [[nodiscard]] GLuint GetTextureViewID(uint8_t layer) const;
        void Render(GlobalLight const *light, BaseCamera const &camera, GLuint trackTextureArrayID,
                    std::vector<std::shared_ptr<Entity>> const &visibleEntities, std::vector<std::shared_ptr<CarAgent>> const &racers);

      private:
        void RenderCascade(uint32_t cascadeIndex, glm::mat4 const &lightSpaceMatrix, GLuint trackTextureArrayID,
                           std::vector<std::shared_ptr<Entity>> const &visibleEntities,
                           std::vector<std::shared_ptr<CarAgent>> const &racers) const;

        GLuint m_depthTextureArrayID = 0;
        std::array<GLuint, CSM_NUM_CASCADES> m_depthTextureViews{};
        DepthShader m_depthShader;
        GLuint m_depthMapFbo = 0;
    };
} // namespace OpenNFS
