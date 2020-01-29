#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "Entity.h"
#include "TrackBlock.h"
#include "VirtualRoad.h"

#include "../Loaders/Shared/CanFile.h"
#include "../Physics/AABBTree.h"
#include "../Renderer/Texture.h"
#include "../Renderer/HermiteCurve.h"

constexpr uint16_t kCullTreeInitialSize = 4000;

class Track
{
public:
    Track() : cullTree(kCullTreeInitialSize), nBlocks(0), tag(UNKNOWN){};
    void GenerateSpline();
    void GenerateAabbTree();

    // Metadata
    NFSVer tag;
    std::string name;
    uint32_t nBlocks;
    std::vector<CameraAnimPoint> cameraAnimation;
    std::vector<VirtualRoad> virtualRoad;
    HermiteCurve centerSpline;

    // Geometry
    std::vector<TrackBlock> trackBlocks;
    std::vector<Entity> globalObjects;
    std::vector<Entity> vroadBarriers;

    // GL 3D Render Data
    std::map<uint32_t, Texture> textureMap;
    GLuint textureArrayID = 0;
    AABBTree cullTree;
};