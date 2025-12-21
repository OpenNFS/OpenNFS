#pragma once

#include <GL/glew.h>
#include <Entities/TrackTextureAsset.h>

#include "../Util/ImageLoader.h"

namespace OpenNFS {
    class GLTexture {
    public:
        GLTexture() = default;
        explicit GLTexture(LibOpenNFS::TrackTextureAsset texture, std::vector<uint8_t> const &data);
        GLubyte *GetData();

        // Utils
        static GLTexture LoadTexture(NFSVersion tag, LibOpenNFS::TrackTextureAsset &trackTexture);
        static int32_t hsStockTextureIndexRemap(int32_t textureIndex);
        static GLuint MakeTextureArray(std::map<uint32_t, GLTexture> &textures, bool repeatable);

        LibOpenNFS::TrackTextureAsset texture_asset;

    private:
        std::vector<GLubyte> data;
    };
} // namespace OpenNFS
