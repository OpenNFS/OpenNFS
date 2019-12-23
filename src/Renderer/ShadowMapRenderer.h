#pragma once

#include <GL/glew.h>
#include "../Scene/GlobalLight.h"
#include "../Loaders/TrackLoader.h"
#include "../RaceNet/CarAgent.h"
#include "../Shaders/DepthShader.h"
#include "../Config.h"

class ShadowMapRenderer {
public:
    ShadowMapRenderer();
    ~ShadowMapRenderer();
    void Render(float nearPlane, float farPlane, const GlobalLight &light, const std::shared_ptr<ONFSTrack> &track, const std::shared_ptr<Car> &car, const std::vector<CarAgent> &racers);

    GLuint m_depthTextureID = 0;
    DepthShader m_depthShader;
private:
    GLuint m_fboDepthMap = 0;
};

