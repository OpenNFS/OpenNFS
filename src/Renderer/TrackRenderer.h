//
// Created by Amrik Sadhra on 21/08/2018.
//

#pragma once

#include "../Scene/Camera.h"
#include "../Shaders/TrackShader.h"
#include "../Shaders/BillboardShader.h"
#include "../Loaders/trk_loader.h"
#include "../Config.h"

class TrackRenderer {
public:
    explicit TrackRenderer(const std::shared_ptr<ONFSTrack> &activeTrack);
    ~TrackRenderer();
    // TODO: Refactor this, we need to pass all cars for spotlights from headlights
    void renderTrack(shared_ptr<Car> &car, const Camera &mainCamera, const Light &sunLight,  std::vector<int> activeTrackBlockIDs, const ParamData &userParams, GLuint depthTextureID, const glm::mat4 &lightSpaceMatrix, float ambientFactor);
    void renderLights(const Camera &mainCamera, std::vector<int> activeTrackBlockIDs);
    static std::vector<Light> GetInterestingLights(const TrackBlock &activeTrackBlock);
    static std::map<int, std::vector<Light>> GetContributingLights(const std::shared_ptr<ONFSTrack> &activeTrack);
    // Create and compile our GLSL programs from the shaders
    TrackShader trackShader;
    BillboardShader billboardShader;
    // Map of lights that contribute to each trackblock
    std::map<int, std::vector<Light>> trackLightMap;
private:
    std::shared_ptr<ONFSTrack> track;
    // Map of COL animated object to anim keyframe
    std::map<int, int> animMap;
};

