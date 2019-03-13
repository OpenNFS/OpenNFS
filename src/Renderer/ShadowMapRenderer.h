//
// Created by amrik on 08/11/18.
//

#pragma once

#include <GL/glew.h>
#include "../Loaders/trk_loader.h"
#include "../RaceNet/CarAgent.h"
#include "../Shaders/DepthShader.h"
#include "../Config.h"

class ShadowMapRenderer {
public:
    explicit ShadowMapRenderer(const std::shared_ptr<ONFSTrack> &activeTrack);
    ~ShadowMapRenderer();
    void renderShadowMap(float nearPlane, float farPlane, const glm::mat4 &lightViewMatrix,  std::vector<int> activeTrackBlockIDs, const std::shared_ptr<Car> &car, const std::vector<CarAgent> &racers);

    GLuint depthTextureID = 0;
    glm::mat4 lightSpaceMatrix;
    DepthShader depthShader;
private:
    std::shared_ptr<ONFSTrack> track;
    GLuint depthMapFBO = 0;
};

