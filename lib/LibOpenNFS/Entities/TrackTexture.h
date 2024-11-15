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
    class TrackTexture {
    public:
        TrackTexture() = default;
        explicit TrackTexture(uint32_t id, uint32_t width, uint32_t height, std::vector<glm::vec2> const& uvs, std::string const& fileReference,
                              std::string const& alphaFileReference);
        std::vector<glm::vec2> GenerateUVs(bool inverseU, bool inverseV, bool reverseOrder);
        std::vector<glm::vec2> GenerateUVs(EntityType meshType, uint32_t textureFlags);

        std::string fileReference;
        std::string alphaFileReference;
        uint32_t id{0};
        uint32_t width{0};
        uint32_t height{0};
        uint32_t layer{0};
        std::vector<glm::vec2> uvs;
        float minU{0.f};
        float minV{0.f};
        float maxU{0.f};
        float maxV{0.f};
    };
} // namespace LibOpenNFS
