//
// Created by Amrik Sadhra on 21/08/2018.
//

#pragma once

#include "../Config.h"
#include "../Loaders/trk_loader.h"
#include "../Scene/Camera.h"
#include "../Shaders/BillboardShader.h"
#include "../Shaders/TrackShader.h"

class TrackRenderer {
  public:
    explicit TrackRenderer(const shared_ptr<ONFSTrack> &activeTrack);
    ~TrackRenderer();
    // TODO: Refactor this, passing Sun and Moon Lights and deriving matrices internally
    void renderTrack(const Camera &mainCamera, const Light &cameraLight, std::vector<int> activeTrackBlockIDs,
                     const ParamData &userParams, uint64_t engineTicks, GLuint depthTextureID,
                     const glm::mat4 &lightSpaceMatrix, float ambientFactor);
    void renderLights(const Camera &mainCamera, std::vector<int> activeTrackBlockIDs);

  private:
    // Create and compile our GLSL programs from the shaders
    TrackShader trackShader;
    BillboardShader billboardShader;
    shared_ptr<ONFSTrack> track;
    // Map of COL animated object to anim keyframe
    std::map<int, int> animMap;
};
