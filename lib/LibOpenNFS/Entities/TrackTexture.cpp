#include "TrackTextureAsset.h"

namespace LibOpenNFS {
    TrackTextureAsset::TrackTextureAsset(uint32_t id, uint32_t width, uint32_t height, std::string const &fileReference, std::string const &alphaFileReference) {
        this->id                 = id;
        this->width              = width;
        this->height             = height;
        this->fileReference      = fileReference;
        this->alphaFileReference = alphaFileReference;
        this->layer              = 0;
        this->minU               = 0.f;
        this->minV               = 0.f;
        this->maxU               = 0.f;
        this->maxV               = 0.f;
    }

    std::vector<glm::vec2> TrackTextureAsset::ScaleUVs(std::vector<glm::vec2> uvs, bool inverseU, bool inverseV) {
        std::vector<glm::vec2> temp_uvs = uvs;
        for (auto &uv : temp_uvs) {
            uv.x = (inverseU ? (1 - uv.x) : uv.x) * maxU;
            uv.y = (inverseV ? (1 - uv.y) : uv.y) * maxV;
        }
        return temp_uvs;
    }
} // namespace LibOpenNFS
