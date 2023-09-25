#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <Models/Track.h>

#include "Entity.h"

#include "../Physics/AABBTree.h"
#include "../Renderer/GLTexture.h"
#include "../Renderer/HermiteCurve.h"

namespace OpenNFS {
    class Track {
        static constexpr uint16_t kCullTreeInitialSize = 4000;

    public:
        Track() : cullTree(kCullTreeInitialSize){};
        ~Track();
        void GenerateSpline();
        void GenerateAabbTree();

        // Metadata
        std::shared_ptr<LibOpenNFS::Track> rawTrack;

        // GL 3D Render Data
        HermiteCurve centerSpline;
        std::map<uint32_t, GLTexture> textureMap;
        GLuint textureArrayID = 0;
        AABBTree cullTree;
    };
} // namespace OpenNFS