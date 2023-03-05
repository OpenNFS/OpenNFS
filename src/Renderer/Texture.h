#pragma once

#include <GL/glew.h>
#include <string>
#include <set>
#include <sstream>
#include <iostream>
#include <bitset>
#include <variant>
#include <filesystem>

#include "../Loaders/NFS2/TRK/ExtraObjectBlock.h"
#include "../Loaders/NFS3/FRD/TexBlock.h"
#include "../Util/Utils.h"
#include "../Util/ImageLoader.h"
#include "../Scene/Entity.h"

// TODO: Refactor this pattern out entirely, should pass everything the texture needs as ONFS intermediate
typedef std::variant<LibOpenNFS::NFS3::TexBlock, LibOpenNFS::NFS2::TEXTURE_BLOCK> RawTextureInfo;

class Texture {
public:
    Texture() = default;
    explicit Texture(NFSVersion tag, uint32_t id, GLubyte *data, uint32_t width, uint32_t height, RawTextureInfo rawTextureInfo);
    std::vector<glm::vec2> GenerateUVs(EntityType meshType, uint32_t textureFlags, RawTextureInfo rawTrackTexture);

    // Utils
    static Texture LoadTexture(NFSVersion tag, RawTextureInfo rawTrackTexture, const std::string &trackName);
    static bool ExtractTrackTextures(const std::string &trackPath, const ::std::string trackName, NFSVersion nfsVer);
    static int32_t hsStockTextureIndexRemap(int32_t textureIndex);
    static GLuint MakeTextureArray(std::map<uint32_t, Texture> &textures, bool repeatable);

    NFSVersion tag;
    uint32_t id, width, height, layer;
    float minU, minV, maxU, maxV;
    GLubyte *data;
    RawTextureInfo rawTextureInfo;
};