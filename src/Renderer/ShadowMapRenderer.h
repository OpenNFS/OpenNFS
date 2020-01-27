#pragma once

#include <GL/glew.h>
#include "../Scene/Lights/GlobalLight.h"
#include "../Loaders/Track.h"
#include "../RaceNet/Agents/CarAgent.h"
#include "../Shaders/DepthShader.h"
#include "../Config.h"

class ShadowMapRenderer {
public:
    ShadowMapRenderer();
    ~ShadowMapRenderer();
    void Render(float nearPlane, float farPlane, const std::shared_ptr<GlobalLight> &light, GLuint trackTextureArrayID, const std::vector<std::shared_ptr<Entity>> &visibleEntities, const std::vector<std::shared_ptr<CarAgent>> &racers);

    GLuint m_depthTextureID = 0;
    DepthShader m_depthShader;
private:
    GLuint m_fboDepthMap = 0;
};

