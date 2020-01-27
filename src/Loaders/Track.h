#pragma once

#include <cstdint>
#include <vector>
#include <memory>


#include "Shared/CanFile.h"
#include "../Renderer/Texture.h"
#include "../Scene/TrackBlock.h"
#include "../Scene/Entity.h"
#include "../Physics/AABBTree.h"
#include "../Renderer/HermiteCurve.h"

constexpr uint16_t kCullTreeInitialSize = 4000;

class Track
{
public:
    Track() : cullTree(kCullTreeInitialSize), nBlocks(0), tag(UNKNOWN) {};
    void GenerateSpline();
    void GenerateAabbTree();

    // Metadata
    NFSVer tag;
    std::string name;
    uint32_t nBlocks;
    std::vector<CameraAnimPoint> cameraAnimation;
    HermiteCurve centerSpline;

    // Geometry
    std::vector<TrackBlock> trackBlocks;
    std::vector<Entity> globalObjects;
    std::vector<Entity> vroadBarriers;

    // GL 3D Render Data
    std::map<uint32_t, Texture> textureMap;
    GLuint textureArrayID;
    AABBTree cullTree;
};