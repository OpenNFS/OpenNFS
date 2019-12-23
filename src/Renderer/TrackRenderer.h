#pragma once

#include "../Scene/Camera.h"
#include "../Scene/GlobalLight.h"
#include "../Shaders/TrackShader.h"
#include "../Shaders/BillboardShader.h"
#include "../Loaders/TrackLoader.h"
#include "../Config.h"

class TrackRenderer {
public:
    explicit TrackRenderer() = default;
    ~TrackRenderer();
    void Render(shared_ptr<Car> &car, const Camera &mainCamera, const GlobalLight &light,  const shared_ptr<ONFSTrack> &track, const ParamData &userParams, GLuint depthTextureID, float ambientFactor);
    void RenderLights(const Camera &mainCamera, const shared_ptr<ONFSTrack> &track);

private:
    // Create and compile our GLSL programs from the shaders
    TrackShader trackShader;
    BillboardShader billboardShader;
};

