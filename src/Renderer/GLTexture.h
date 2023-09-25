#pragma once

#include <GL/glew.h>
#include <string>
#include <set>
#include <sstream>
#include <iostream>
#include <bitset>
#include <variant>
#include <filesystem>
#include <Models/Texture.h>

#include <NFS2/TRK/ExtraObjectBlock.h>
#include <NFS3/FRD/TexBlock.h>
#include "../Util/Utils.h"
#include "../Util/ImageLoader.h"
#include "../Scene/Entity.h"


class GLTexture {
public:
    GLTexture() = default;
    explicit GLTexture(LibOpenNFS::Texture texture, GLubyte *data);

    // Utils
    static GLTexture LoadTexture(NFSVersion tag, const LibOpenNFS::Texture &rawTrackTexture, const std::string &trackName);
    static bool ExtractTrackTextures(const std::string &trackPath, const ::std::string &trackName, NFSVersion nfsVer);
    static int32_t hsStockTextureIndexRemap(int32_t textureIndex);
    static GLuint MakeTextureArray(std::map<uint32_t, GLTexture> &textures, bool repeatable);

    LibOpenNFS::Texture texture;
    GLubyte *data;
};