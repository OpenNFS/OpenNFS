#pragma once

#include "../Camera/BaseCamera.h"
#include <Common/Lights/BaseLight.h>
#include <Common/Lights/GlobalLight.h>
#include "../Shaders/TrackShader.h"
#include "../Shaders/BillboardShader.h"
#include "../RaceNet/Agents/CarAgent.h"
#include "../Config.h"

class TrackRenderer {
public:
    explicit TrackRenderer() = default;
    ~TrackRenderer();
    void Render(const std::vector<std::shared_ptr<CarAgent>> &racers,
                const BaseCamera &camera,
                GLuint trackTextureArrayID,
                const std::vector<std::shared_ptr<Entity>> &visibleEntities,
                const std::vector<shared_ptr<BaseLight>> &lights,
                const ParamData &userParams,
                GLuint depthTextureID,
                float ambientFactor);
    void RenderLights(const BaseCamera &camera, const std::vector<shared_ptr<BaseLight>> &lights);

private:
    // Create and compile our GLSL programs from the shaders
    TrackShader m_trackShader;
    BillboardShader m_billboardShader;
};
