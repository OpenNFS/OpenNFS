#pragma once

#include <bitset>
#include <filesystem>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "TrackEntity.h"
#include <NFS2/TRK/ExtraObjectBlock.h>
#include <NFS3/FRD/TexBlock.h>

namespace LibOpenNFS {
    // TODO: Refactor this pattern out entirely, should pass everything the texture needs as ONFS intermediate
    typedef std::variant<LibOpenNFS::NFS3::TexBlock, LibOpenNFS::NFS2::TEXTURE_BLOCK> RawTextureInfo;
    class Texture {
    public:
        Texture() = default;
        explicit Texture(NFSVersion tag, uint32_t id, const std::vector<uint8_t> &data, uint32_t width, uint32_t height, RawTextureInfo rawTextureInfo);
        std::vector<glm::vec2> GenerateUVs(EntityType meshType, uint32_t textureFlags);
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        bool    IsLane() const;
        uint32_t GetTextureID() const;

        NFSVersion tag;
        uint32_t id, width, height, layer;
        float minU, minV, maxU, maxV;
    private:
        RawTextureInfo rawTextureInfo;
        std::vector<uint8_t> data;
    };
} // namespace LibOpenNFS
