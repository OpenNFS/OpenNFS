#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <Entities/Track.h>

#include "Entity.h"

#include "../Physics/AABBTree.h"
#include "../Renderer/GLTexture.h"
#include "../Renderer/HermiteCurve.h"

namespace OpenNFS {
    class Track : public LibOpenNFS::Track {
        static constexpr uint16_t kCullTreeInitialSize = 4000;

    public:
        explicit Track(const LibOpenNFS::Track &track);
        Track() : cullTree(kCullTreeInitialSize){};
        ~Track();

        // GL 3D Render Data
        HermiteCurve centerSpline;

    private:
        void _LoadTextures();
        void _GenerateSpline();
        void _GenerateAabbTree();

        std::map<uint32_t, GLTexture> textureMap;
        GLuint textureArrayID = 0;
        AABBTree cullTree;
    };
} // namespace OpenNFS