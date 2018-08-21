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
    explicit TrackRenderer(const shared_ptr<ONFSTrack> &activeTrack);
    ~TrackRenderer();
    void renderTrack(const Camera &mainCamera, const Light &cameraLight, std::vector<int> activeTrackBlockIDs, ParamData userParams);
    void renderLights(const Camera &mainCamera, std::vector<int> activeTrackBlockIDs);
private:
    // Create and compile our GLSL programs from the shaders
    TrackShader trackShader;
    BillboardShader billboardShader;
    shared_ptr<ONFSTrack> track;
    // Map of COL animated object to anim keyframe
    std::map<int, int> animMap;
};

