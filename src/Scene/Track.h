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
        GLuint textureArrayID = 0;
        std::vector<std::shared_ptr<Entity>> entities;
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