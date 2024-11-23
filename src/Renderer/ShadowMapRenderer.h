#pragma once

#include <GL/glew.h>

#include "../Config.h"
#include "../Scene/GlobalLight.h"
#include "../Scene/Track.h"
#include "../Shaders/DepthShader.h"
#include "../RaceNet/Agents/CarAgent.h"

namespace OpenNFS {
    class ShadowMapRenderer {
    public:
        ShadowMapRenderer();
        ~ShadowMapRenderer();
        void Render(float nearPlane,
                    float farPlane,
                    const GlobalLight *light,
                    GLuint trackTextureArrayID,
                    const std::vector<std::shared_ptr<Entity>> &visibleEntities,
                    const std::vector<std::shared_ptr<CarAgent>> &racers);

        GLuint m_depthTextureID = 0;
        DepthShader m_depthShader;

    private:
        GLuint m_fboDepthMap = 0;
    };
} // namespace OpenNFS
