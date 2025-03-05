#pragma once

#include <cstdint>
#include <vector>
#include <Entities/Track.h>

#include "Entity.h"
#include "../Physics/AABBTree.h"
#include "../Renderer/GLTexture.h"
#include "../Renderer/HermiteCurve.h"

namespace OpenNFS {
    class Track : public LibOpenNFS::Track {
        static constexpr uint32_t kCullTreeInitialSize{100000};

      public:
        explicit Track(const LibOpenNFS::Track &track);
        Track() : cullTree(kCullTreeInitialSize){};
        ~Track();

        // GL 3D Render Data
        HermiteCurve centerSpline;
        GLuint textureArrayID {0};
        std::vector<std::vector<std::shared_ptr<Entity>>> perTrackblockEntities; // [Trackblock 0: [entity 0, entity 1, ...], Trackblock 1: [entity X, ...]]
        AABBTree cullTree;

    private:
        void _LoadTextures();
        void _GenerateEntities();
        void _GenerateSpline();
        void _GenerateAabbTree();

        std::string assetPath;
        std::map<uint32_t, GLTexture> textureMap;
    };
} // namespace OpenNFS