#include "Texture.h"

#include "Texture.h"

namespace LibOpenNFS {
    Texture::Texture(NFSVersion tag, uint32_t id, const std::vector<uint8_t> &data, uint32_t width, uint32_t height, RawTextureInfo rawTextureInfo) {
        this->tag            = tag;
        this->id             = id;
        this->data           = data;
        this->width          = width;
        this->height         = height;
        this->layer          = 0;
        this->minU           = 0.f;
        this->minV           = 0.f;
        this->maxU           = 0.f;
        this->maxV           = 0.f;
        this->rawTextureInfo = rawTextureInfo;
    }

    uint32_t Texture::GetWidth() const {
        switch (tag) {
        case NFSVersion::UNKNOWN:
            break;
        case NFSVersion::NFS_1:
            break;
        case NFSVersion::NFS_2:
            break;
        case NFSVersion::NFS_2_PS1:
            break;
        case NFSVersion::NFS_2_SE:
            break;
        case NFSVersion::NFS_3:
            return std::get<LibOpenNFS::NFS3::TexBlock>(rawTextureInfo).width;
        case NFSVersion::NFS_3_PS1:
            break;
        case NFSVersion::NFS_4:
            break;
        case NFSVersion::NFS_4_PS1:
            break;
        case NFSVersion::MCO:
            break;
        case NFSVersion::NFS_5:
            break;
        }
        return 0;
    }

    uint32_t Texture::GetHeight() const {
        switch (tag) {
        case NFSVersion::UNKNOWN:
            break;
        case NFSVersion::NFS_1:
            break;
        case NFSVersion::NFS_2:
            break;
        case NFSVersion::NFS_2_PS1:
            break;
        case NFSVersion::NFS_2_SE:
            break;
        case NFSVersion::NFS_3:
            return std::get<LibOpenNFS::NFS3::TexBlock>(rawTextureInfo).height;
        case NFSVersion::NFS_3_PS1:
            break;
        case NFSVersion::NFS_4:
            break;
        case NFSVersion::NFS_4_PS1:
            break;
        case NFSVersion::MCO:
            break;
        case NFSVersion::NFS_5:
            break;
        }
        return 0;
    }

    bool Texture::IsLane() const {
        switch (tag) {
        case NFSVersion::UNKNOWN:
            break;
        case NFSVersion::NFS_1:
            break;
        case NFSVersion::NFS_2:
            break;
        case NFSVersion::NFS_2_PS1:
            break;
        case NFSVersion::NFS_2_SE:
            break;
        case NFSVersion::NFS_3:
            return std::get<LibOpenNFS::NFS3::TexBlock>(rawTextureInfo).isLane;
        case NFSVersion::NFS_3_PS1:
            break;
        case NFSVersion::NFS_4:
            break;
        case NFSVersion::NFS_4_PS1:
            break;
        case NFSVersion::MCO:
            break;
        case NFSVersion::NFS_5:
            break;
        }
        return false;
    }

    uint32_t Texture::GetTextureID() const {
        switch (tag) {
        case NFSVersion::UNKNOWN:
            break;
        case NFSVersion::NFS_1:
            break;
        case NFSVersion::NFS_2:
        case NFSVersion::NFS_2_PS1:
        case NFSVersion::NFS_2_SE:
        case NFSVersion::NFS_3_PS1:
            return std::get<LibOpenNFS::NFS2::TEXTURE_BLOCK>(rawTextureInfo).texNumber;
        case NFSVersion::NFS_3:
            return std::get<LibOpenNFS::NFS3::TexBlock>(rawTextureInfo).qfsIndex;
        case NFSVersion::NFS_4:
            break;
        case NFSVersion::NFS_4_PS1:
            break;
        case NFSVersion::MCO:
            break;
        case NFSVersion::NFS_5:
            break;
        }
        return 0;
    }

    std::vector<glm::vec2> Texture::GenerateUVs(EntityType meshType, uint32_t textureFlags) {
        std::bitset<32> textureAlignment(textureFlags);
        std::vector<glm::vec2> uvs;

        switch (tag) {
        case NFSVersion::NFS_1:
            ASSERT(false, "Unimplemented");
            break;
        case NFSVersion::NFS_2:
        case NFSVersion::NFS_2_SE:
        case NFSVersion::NFS_2_PS1:
        case NFSVersion::NFS_3_PS1:
            switch (meshType) {
            case EntityType::XOBJ: {
                bool horizontalFlip           = false; // textureAlignment[8];
                bool verticalFlip             = false; // textureAlignment[9];
                glm::vec2 originTransform     = glm::vec2(0.5f, 0.5f);
                uint8_t nRotate               = 0;
                float angle                   = nRotate * 90.f;
                glm::mat2 uvRotationTransform = glm::mat2(cos(glm::radians(angle)), sin(glm::radians(angle)), -sin(glm::radians(angle)), cos(glm::radians(angle)));
                uvs.emplace_back(((glm::vec2(1.0f, 1.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(0.0f, 1.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(0.0f, 0.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(1.0f, 1.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(0.0f, 0.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(1.0f, 0.0f) - originTransform) * uvRotationTransform) + originTransform);
                for (auto &uv : uvs) {
                    if (horizontalFlip) {
                        uv.x = 1.0f - uv.x;
                    }
                    if (verticalFlip) {
                        uv.y = 1.0f - uv.y;
                    }
                    uv.x *= maxU;
                    uv.y *= maxV;
                }
            } break;
            case EntityType::OBJ_POLY:
                break;
            case EntityType::ROAD: {
                bool horizontalFlip           = false; // textureAlignment[8];
                bool verticalFlip             = false; // textureAlignment[9];
                glm::vec2 originTransform     = glm::vec2(0.5f, 0.5f);
                uint8_t nRotate               = (textureFlags >> 11) & 3; // 8,11 ok
                float angle                   = nRotate * 90.f;
                glm::mat2 uvRotationTransform = glm::mat2(cos(glm::radians(angle)), sin(glm::radians(angle)), -sin(glm::radians(angle)), cos(glm::radians(angle)));
                uvs.emplace_back(((glm::vec2(1.0f, 1.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(0.0f, 1.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(0.0f, 0.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(1.0f, 1.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(0.0f, 0.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(1.0f, 0.0f) - originTransform) * uvRotationTransform) + originTransform);
                for (auto &uv : uvs) {
                    if (horizontalFlip) {
                        uv.x = 1.0f - uv.x;
                    }
                    if (verticalFlip) {
                        uv.y = 1.0f - uv.y;
                    }
                    uv.x *= this->maxU;
                    uv.y *= this->maxV;
                }
            } break;
            case EntityType::GLOBAL:
                break;
            case EntityType::CAR:
                break;
            case EntityType::LANE:
                break;
            case EntityType::SOUND:
                break;
            case EntityType::LIGHT:
                break;
            case EntityType::VROAD:
                break;
            case EntityType::VROAD_CEIL:
                break;
            }
            break;
        /*    switch (meshType)
            {
            case XOBJ:
                uvs.emplace_back(1.0f * maxU, 1.0f * maxV);
                uvs.emplace_back(0.0f * maxU, 1.0f * maxV);
                uvs.emplace_back(0.0f * maxU, 0.0f * maxV);
                uvs.emplace_back(1.0f * maxU, 1.0f * maxV);
                uvs.emplace_back(0.0f * maxU, 0.0f * maxV);
                uvs.emplace_back(1.0f * maxU, 0.0f * maxV);
                break;
            case OBJ_POLY:
                break;
            case ROAD:
                uvs.emplace_back(1.0f * maxU, 1.0f * maxV);
                uvs.emplace_back(0.0f * maxU, 1.0f * maxV);
                uvs.emplace_back(0.0f * maxU, 0.0f * maxV);
                uvs.emplace_back(1.0f * maxU, 1.0f * maxV);
                uvs.emplace_back(0.0f * maxU, 0.0f * maxV);
                uvs.emplace_back(1.0f * maxU, 0.0f * maxV);
                break;
            case GLOBAL:
                break;
            case CAR:
            {
                bool horizontalFlip           = textureAlignment[4];
                bool verticalFlip             = true;
                glm::vec2 originTransform     = glm::vec2(0.5f, 0.5f);
                uint8_t nRotate               = static_cast<uint8_t>((textureFlags) &3);
                float angle                   = nRotate * 90.f;
                glm::mat2 uvRotationTransform = glm::mat2(cos(glm::radians(angle)), sin(glm::radians(angle)), -sin(glm::radians(angle)), cos(glm::radians(angle)));
                uvs.emplace_back(((glm::vec2(1.0f, 1.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(0.0f, 1.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(0.0f, 0.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(1.0f, 1.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(0.0f, 0.0f) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(1.0f, 0.0f) - originTransform) * uvRotationTransform) + originTransform);
                for (auto &uv : uvs)
                {
                    if (horizontalFlip)
                    {
                        uv.x = 1.0f - uv.x;
                    }
                    if (verticalFlip)
                    {
                        uv.y = 1.0f - uv.y;
                    }
                    uv.x *= maxU;
                    uv.y *= maxV;
                }
            }
            break;
            }
            break;*/
        case NFSVersion::NFS_3: {
            LibOpenNFS::NFS3::TexBlock texBlock = std::get<LibOpenNFS::NFS3::TexBlock>(rawTextureInfo);
            switch (meshType) {
            case EntityType::XOBJ:
                uvs.emplace_back((1.0f - texBlock.corners[0]) * maxU, (1.0f - texBlock.corners[1]) * maxV);
                uvs.emplace_back((1.0f - texBlock.corners[2]) * maxU, (1.0f - texBlock.corners[3]) * maxV);
                uvs.emplace_back((1.0f - texBlock.corners[4]) * maxU, (1.0f - texBlock.corners[5]) * maxV);
                uvs.emplace_back((1.0f - texBlock.corners[0]) * maxU, (1.0f - texBlock.corners[1]) * maxV);
                uvs.emplace_back((1.0f - texBlock.corners[4]) * maxU, (1.0f - texBlock.corners[5]) * maxV);
                uvs.emplace_back((1.0f - texBlock.corners[6]) * maxU, (1.0f - texBlock.corners[7]) * maxV);
                break;
            case EntityType::OBJ_POLY:
            case EntityType::LANE:
            case EntityType::ROAD:
                uvs.emplace_back(texBlock.corners[0] * maxU, (1.0f - texBlock.corners[1]) * maxV);
                uvs.emplace_back(texBlock.corners[2] * maxU, (1.0f - texBlock.corners[3]) * maxV);
                uvs.emplace_back(texBlock.corners[4] * maxU, (1.0f - texBlock.corners[5]) * maxV);
                uvs.emplace_back(texBlock.corners[0] * maxU, (1.0f - texBlock.corners[1]) * maxV);
                uvs.emplace_back(texBlock.corners[4] * maxU, (1.0f - texBlock.corners[5]) * maxV);
                uvs.emplace_back(texBlock.corners[6] * maxU, (1.0f - texBlock.corners[7]) * maxV);
                break;
            case EntityType::GLOBAL:
                break;
            case EntityType::CAR:
                break;
            case EntityType::SOUND:
                break;
            case EntityType::LIGHT:
                break;
            case EntityType::VROAD:
                break;
            case EntityType::VROAD_CEIL:
                break;
            }
        } break;
        case NFSVersion::NFS_4: {
            // TODO: Needs to be an NFS4 texblock after NFS4 new gen parser bringup
            LibOpenNFS::NFS3::TexBlock texBlock = std::get<LibOpenNFS::NFS3::TexBlock>(rawTextureInfo);
            switch (meshType) {
            //(flags>>2)&3 indicates the multiple of 90° by which the
            // texture should be rotated (0 for no rotation, 1 for 90°,
            // 2 for 180°, 3 for 270°) ; a non-zero value of flags&0x10
            // indicates that the texture is horizontally flipped ; a
            // non-zero value of flags&0x20 indicates that the texture
            // is vertically flipped. The value of (flags>>6)&7 indicates
            // the scaling factor : 0 is no scaling ; 1 means that the
            // texture is tiled twice horizontally ; 2 that the texture
            // is tile twice vertically ; 3 indicates 4x horizontal
            // tiling, 4 indicates 4x vertical tiling. Finally, a non-zero
            // value of flags&0x8000 indicates that the polygon is one-sided.
            // ox, oy, and oz :: Origin of the wrap.
            // dx, dy, and dz :: The z-axis of the wrap.
            // ux, uy, and uz ::	The y-axis of the wrap.
            // ou and ov :: Origin in the texture.
            // su and sv :: Scale factor in the texture
            case EntityType::XOBJ: {
                bool horizontalFlip           = textureAlignment[4];
                bool verticalFlip             = textureAlignment[5];
                glm::vec2 originTransform     = glm::vec2(0.5f, 0.5f);
                uint8_t nRotate               = static_cast<uint8_t>((textureFlags >> 2) & 3);
                float angle                   = nRotate * 90.f;
                glm::mat2 uvRotationTransform = glm::mat2(cos(glm::radians(angle)), sin(glm::radians(angle)), -sin(glm::radians(angle)), cos(glm::radians(angle)));
                uvs.emplace_back(((glm::vec2(1.0f - texBlock.corners[0], 1.0f - texBlock.corners[1]) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(1.0f - texBlock.corners[2], 1.0f - texBlock.corners[3]) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(1.0f - texBlock.corners[4], 1.0f - texBlock.corners[5]) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(1.0f - texBlock.corners[0], 1.0f - texBlock.corners[1]) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(1.0f - texBlock.corners[4], 1.0f - texBlock.corners[5]) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(1.0f - texBlock.corners[6], 1.0f - texBlock.corners[7]) - originTransform) * uvRotationTransform) + originTransform);
                for (auto &uv : uvs) {
                    if (horizontalFlip) {
                        uv.x = 1.0f - uv.x;
                    }
                    if (verticalFlip) {
                        uv.y = 1.0f - uv.y;
                    }
                    uv.x *= maxU;
                    uv.y *= maxV;
                }
            } break;
            case EntityType::OBJ_POLY:
            case EntityType::ROAD:
            case EntityType::LANE:
            case EntityType::GLOBAL: {
                bool horizontalFlip           = textureAlignment[4];
                bool verticalFlip             = textureAlignment[5];
                glm::vec2 originTransform     = glm::vec2(0.5f, 0.5f);
                uint8_t nRotate               = static_cast<uint8_t>((textureFlags >> 2) & 3);
                float angle                   = nRotate * 90.f;
                glm::mat2 uvRotationTransform = glm::mat2(cos(glm::radians(angle)), sin(glm::radians(angle)), -sin(glm::radians(angle)), cos(glm::radians(angle)));
                uvs.emplace_back(((glm::vec2(texBlock.corners[0], 1.0f - texBlock.corners[1]) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(texBlock.corners[2], 1.0f - texBlock.corners[3]) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(texBlock.corners[4], 1.0f - texBlock.corners[5]) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(texBlock.corners[0], 1.0f - texBlock.corners[1]) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(texBlock.corners[4], 1.0f - texBlock.corners[5]) - originTransform) * uvRotationTransform) + originTransform);
                uvs.emplace_back(((glm::vec2(texBlock.corners[6], 1.0f - texBlock.corners[7]) - originTransform) * uvRotationTransform) + originTransform);
                for (auto &uv : uvs) {
                    if (horizontalFlip) {
                        uv.x = 1.0f - uv.x;
                    }
                    if (verticalFlip) {
                        uv.y = 1.0f - uv.y;
                    }
                    uv.x *= maxU;
                    uv.y *= maxV;
                }
            } break;
            case EntityType::CAR:
                break;
            case EntityType::SOUND:
                break;
            case EntityType::LIGHT:
                break;
            case EntityType::VROAD:
                break;
            case EntityType::VROAD_CEIL:
                break;
            }
        } break;
        case NFSVersion::NFS_5:
            break;
        case NFSVersion::UNKNOWN:
            break;
        case NFSVersion::NFS_4_PS1:
            break;
        case NFSVersion::MCO:
            break;
        }

        return uvs;
    }
} // namespace LibOpenNFS
