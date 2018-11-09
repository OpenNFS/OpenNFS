//
// Created by amrik on 08/11/18.
//

#pragma once

#include <GL/glew.h>
#include "../Loaders/trk_loader.h"
#include "../Shaders/DepthShader.h"

class ShadowMapRenderer {
public:
    explicit ShadowMapRenderer(const shared_ptr<ONFSTrack> &activeTrack);
    ~ShadowMapRenderer();
    void renderShadowMap(const glm::mat4 &lightViewMatrix,  std::vector<int> activeTrackBlockIDs, const std::shared_ptr<Car> &car);

    GLuint depthTextureID = 0;
    glm::mat4 lightSpaceMatrix;
private:
    shared_ptr<ONFSTrack> track;
    /* Shadow Mapping */
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    GLuint depthMapFBO = 0;
    DepthShader depthShader;
};

