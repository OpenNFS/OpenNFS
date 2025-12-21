#pragma once

#include <GL/glew.h>

#include "../Race/Agents/CarAgent.h"
#include "../Scene/GlobalLight.h"
#include "../Scene/Track.h"
#include "Shaders/DepthShader.h"

namespace OpenNFS {
    class ShadowMapRenderer {
      public:
        ShadowMapRenderer();
        ~ShadowMapRenderer();
        [[nodiscard]] GLuint GetTextureID() const;
        void Render(float nearPlane,
                    float farPlane,
                    GlobalLight const *light,
                    GLuint trackTextureArrayID,
                    std::vector<std::shared_ptr<Entity>> const &visibleEntities,
                    std::vector<std::shared_ptr<CarAgent>> const &racers);

      private:
        GLuint m_depthTextureID = 0;
        DepthShader m_depthShader;
        GLuint m_depthMapFbo = 0;
    };
} // namespace OpenNFS
