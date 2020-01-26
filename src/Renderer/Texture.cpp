#include "Texture.h"

Texture::Texture(uint32_t id,  GLubyte *data, uint32_t width, uint32_t height)
{
    this->id = id;
    this->data = data;
    this->width = width;
    this->height = height;
    this->min_u = 0.f;
    this->min_v = 0.f;
    this->max_u = 0.f;
    this->max_v = 0.f;
}

bool ExtractTrackTextures(const std::string &track_path, const ::std::string track_name, NFSVer nfs_version) {
    std::stringstream output_dir, tex_archive_path;
    std::string psh_path = track_path;
    std::string full_track_path = track_path + "/" + track_name;
    output_dir << TRACK_PATH << ToString(nfs_version) << "/";

    switch (nfs_version) {
        case NFS_2:
            tex_archive_path << track_path << "0.qfs";
            break;
        case NFS_2_SE:
            tex_archive_path << track_path << "0M.qfs";
            break;
        case NFS_3:
            tex_archive_path << full_track_path << "0.qfs";
            break;
        case NFS_3_PS1:
            psh_path.replace(psh_path.find("ZZ"), 2, "");
            tex_archive_path << psh_path << "0.PSH";
            break;
        case NFS_4:
            tex_archive_path << track_path << "/TR0.qfs";
            break;
        case UNKNOWN:
        default:
            ASSERT(false, "Trying to extract track textures from unknown NFS version");
            break;
    }
    output_dir << track_name;

    if (boost::filesystem::exists(output_dir.str())) {
        return true;
    } else {
        boost::filesystem::create_directories(output_dir.str());
    }

    LOG(INFO) << "Extracting track textures";

    if (nfs_version == NFS_3_PS1) {
        output_dir << "/textures/";
        return ImageLoader::ExtractPSH(tex_archive_path.str(), output_dir.str());
    } else if (nfs_version == NFS_3) {
        std::stringstream sky_fsh_path;
        sky_fsh_path << full_track_path.substr(0, full_track_path.find_last_of('/')) << "/sky.fsh";
        if (boost::filesystem::exists(sky_fsh_path.str())) {
            std::stringstream sky_textures_path;
            sky_textures_path << output_dir.str() << "/sky_textures/";
            ASSERT(ImageLoader::ExtractQFS(sky_fsh_path.str(), sky_textures_path.str()), "Unable to extract sky textures from " << sky_fsh_path.str());
        }
    }

    output_dir << "/textures/";
    return (ImageLoader::ExtractQFS(tex_archive_path.str(), output_dir.str()));
}

int hsStockTextureIndexRemap(int textureIndex) {
    int remappedIndex = textureIndex;

    int nStockTextures = 30;

    // Remap texture index between 0 and MAX_TEXTURE_ARRAY_SIZE if exceeds
    if (textureIndex >= 2048) {
        remappedIndex = MAX_TEXTURE_ARRAY_SIZE - nStockTextures + (textureIndex - 2048);
    }

    ASSERT(remappedIndex <= MAX_TEXTURE_ARRAY_SIZE, "Texture index still exceeds max texture array size, post-remap");

    return remappedIndex;
}

std::vector<glm::vec2> Texture::GenerateUVs(NFSVer tag, EntityType mesh_type, uint32_t textureFlags) {
    std::bitset<32> textureAlignment(textureFlags);
    std::vector<glm::vec2> uvs;

    switch (tag) {
        case NFS_1:
            ASSERT(false, "Unimplemented");
            break;
        case NFS_2:
        case NFS_2_SE:
            switch (mesh_type) {
                case XOBJ: {
                    bool horizontalFlip = false; //textureAlignment[8];
                    bool verticalFlip = false; //textureAlignment[9];
                    glm::vec2 originTransform = glm::vec2(0.5f, 0.5f);
                    uint8_t nRotate = 0;
                    float angle = nRotate * 90.f;
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
                        uv.x *= this->max_u;
                        uv.y *= this->max_v;
                    }
                }
                    break;
                case OBJ_POLY:
                    break;
                case ROAD:
                {
                    bool horizontalFlip = false; // textureAlignment[8];
                    bool verticalFlip = false;   // textureAlignment[9];
                    glm::vec2 originTransform = glm::vec2(0.5f, 0.5f);
                    uint8_t nRotate = (textureFlags >> 11) & 3; // 8,11 ok
                    float angle = nRotate * 90.f;
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
                        uv.x *= this->max_u;
                        uv.y *= this->max_v;
                    }
                }
                    break;
                case GLOBAL:
                    break;
                case CAR:
                    break;
            }
            break;
        case NFS_2_PS1:
        case NFS_3_PS1:
            switch (mesh_type) {
                case XOBJ:
                    uvs.emplace_back(1.0f * this->max_u, 1.0f * this->max_v);
                    uvs.emplace_back(0.0f * this->max_u, 1.0f * this->max_v);
                    uvs.emplace_back(0.0f * this->max_u, 0.0f * this->max_v);
                    uvs.emplace_back(1.0f * this->max_u, 1.0f * this->max_v);
                    uvs.emplace_back(0.0f * this->max_u, 0.0f * this->max_v);
                    uvs.emplace_back(1.0f * this->max_u, 0.0f * this->max_v);
                    break;
                case OBJ_POLY:
                    break;
                case ROAD:
                    uvs.emplace_back(1.0f * this->max_u, 1.0f * this->max_v);
                    uvs.emplace_back(0.0f * this->max_u, 1.0f * this->max_v);
                    uvs.emplace_back(0.0f * this->max_u, 0.0f * this->max_v);
                    uvs.emplace_back(1.0f * this->max_u, 1.0f * this->max_v);
                    uvs.emplace_back(0.0f * this->max_u, 0.0f * this->max_v);
                    uvs.emplace_back(1.0f * this->max_u, 0.0f * this->max_v);
                    break;
                case GLOBAL:
                    break;
                case CAR: {
                    bool horizontalFlip = textureAlignment[4];
                    bool verticalFlip = true;
                    glm::vec2 originTransform = glm::vec2(0.5f, 0.5f);
                    uint8_t nRotate = static_cast<uint8_t>((textureFlags) & 3);
                    float angle = nRotate * 90.f;
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
                        uv.x *= this->max_u;
                        uv.y *= this->max_v;
                    }
                }
                    break;
            }
            break;
        case NFS_3:
        case NFS_4:
            ASSERT(false, "Incorrect UV generate function call, did you mean to pass in an NFS3_4::TEXTUREBLOCK as well?");
            break;
        case NFS_5:
            ASSERT(false, "Unimplemented");
            break;
        case UNKNOWN:
            ASSERT(false, "Unimplemented");
            break;
    }

    return uvs;
}

std::vector<glm::vec2> GenerateUVs(NFSVer tag, EntityType mesh_type, uint32_t textureFlags, Texture gl_texture, TexBlock texture_block) {
    std::bitset<32> textureAlignment(textureFlags);
    std::vector<glm::vec2> uvs;

    switch (tag) {
        case NFS_1:
        case NFS_2:
        case NFS_2_SE:
        case NFS_2_PS1:
        case NFS_3_PS1:
            ASSERT(false, "Incorrect UV generate function call, did you mean to pass in an NFS3_4::TEXTUREBLOCK as well?");
            break;
        case NFS_3:
            switch (mesh_type) {
                case XOBJ:
                    uvs.emplace_back((1.0f - texture_block.corners[0]) * gl_texture.max_u, (1.0f - texture_block.corners[1]) * gl_texture.max_v);
                    uvs.emplace_back((1.0f - texture_block.corners[2]) * gl_texture.max_u, (1.0f - texture_block.corners[3]) * gl_texture.max_v);
                    uvs.emplace_back((1.0f - texture_block.corners[4]) * gl_texture.max_u, (1.0f - texture_block.corners[5]) * gl_texture.max_v);
                    uvs.emplace_back((1.0f - texture_block.corners[0]) * gl_texture.max_u, (1.0f - texture_block.corners[1]) * gl_texture.max_v);
                    uvs.emplace_back((1.0f - texture_block.corners[4]) * gl_texture.max_u, (1.0f - texture_block.corners[5]) * gl_texture.max_v);
                    uvs.emplace_back((1.0f - texture_block.corners[6]) * gl_texture.max_u, (1.0f - texture_block.corners[7]) * gl_texture.max_v);
                    break;
                case OBJ_POLY:
                case LANE:
                case ROAD:
                    uvs.emplace_back(texture_block.corners[0] * gl_texture.max_u, (1.0f - texture_block.corners[1]) * gl_texture.max_v);
                    uvs.emplace_back(texture_block.corners[2] * gl_texture.max_u, (1.0f - texture_block.corners[3]) * gl_texture.max_v);
                    uvs.emplace_back(texture_block.corners[4] * gl_texture.max_u, (1.0f - texture_block.corners[5]) * gl_texture.max_v);
                    uvs.emplace_back(texture_block.corners[0] * gl_texture.max_u, (1.0f - texture_block.corners[1]) * gl_texture.max_v);
                    uvs.emplace_back(texture_block.corners[4] * gl_texture.max_u, (1.0f - texture_block.corners[5]) * gl_texture.max_v);
                    uvs.emplace_back(texture_block.corners[6] * gl_texture.max_u, (1.0f - texture_block.corners[7]) * gl_texture.max_v);
                    break;
                case GLOBAL:
                    break;
                case CAR:
                    break;
            }

            break;
        case NFS_4:
            switch (mesh_type) {
                //(flags>>2)&3 indicates the multiple of 90° by which the
                //texture should be rotated (0 for no rotation, 1 for 90°,
                //2 for 180°, 3 for 270°) ; a non-zero value of flags&0x10
                //indicates that the texture is horizontally flipped ; a
                //non-zero value of flags&0x20 indicates that the texture
                //is vertically flipped. The value of (flags>>6)&7 indicates
                //the scaling factor : 0 is no scaling ; 1 means that the
                //texture is tiled twice horizontally ; 2 that the texture
                //is tile twice vertically ; 3 indicates 4x horizontal
                //tiling, 4 indicates 4x vertical tiling. Finally, a non-zero
                //value of flags&0x8000 indicates that the polygon is one-sided.
                //ox, oy, and oz :: Origin of the wrap.
                //dx, dy, and dz :: The z-axis of the wrap.
                //ux, uy, and uz ::	The y-axis of the wrap.
                //ou and ov :: Origin in the texture.
                //su and sv :: Scale factor in the texture
                case XOBJ: {
                    bool horizontalFlip = textureAlignment[4];
                    bool verticalFlip = textureAlignment[5];
                    glm::vec2 originTransform = glm::vec2(0.5f, 0.5f);
                    uint8_t nRotate = static_cast<uint8_t>((textureFlags >> 2) & 3);
                    float angle = nRotate * 90.f;
                    glm::mat2 uvRotationTransform = glm::mat2(cos(glm::radians(angle)), sin(glm::radians(angle)), -sin(glm::radians(angle)), cos(glm::radians(angle)));
                    uvs.emplace_back(((glm::vec2(1.0f - texture_block.corners[0], 1.0f - texture_block.corners[1]) - originTransform) * uvRotationTransform) + originTransform);
                    uvs.emplace_back(((glm::vec2(1.0f - texture_block.corners[2], 1.0f - texture_block.corners[3]) - originTransform) * uvRotationTransform) + originTransform);
                    uvs.emplace_back(((glm::vec2(1.0f - texture_block.corners[4], 1.0f - texture_block.corners[5]) - originTransform) * uvRotationTransform) + originTransform);
                    uvs.emplace_back(((glm::vec2(1.0f - texture_block.corners[0], 1.0f - texture_block.corners[1]) - originTransform) * uvRotationTransform) + originTransform);
                    uvs.emplace_back(((glm::vec2(1.0f - texture_block.corners[4], 1.0f - texture_block.corners[5]) - originTransform) * uvRotationTransform) + originTransform);
                    uvs.emplace_back(((glm::vec2(1.0f - texture_block.corners[6], 1.0f - texture_block.corners[7]) - originTransform) * uvRotationTransform) + originTransform);
                    for (auto &uv : uvs) {
                        if (horizontalFlip) {
                            uv.x = 1.0f - uv.x;
                        }
                        if (verticalFlip) {
                            uv.y = 1.0f - uv.y;
                        }
                        uv.x *= gl_texture.max_u;
                        uv.y *= gl_texture.max_v;
                    }
                }
                    break;
                case OBJ_POLY:
                case ROAD:
                case LANE:
                case GLOBAL: {
                    bool horizontalFlip = textureAlignment[4];
                    bool verticalFlip = textureAlignment[5];
                    glm::vec2 originTransform = glm::vec2(0.5f, 0.5f);
                    uint8_t nRotate = static_cast<uint8_t>((textureFlags >> 2) & 3);
                    float angle = nRotate * 90.f;
                    glm::mat2 uvRotationTransform = glm::mat2(cos(glm::radians(angle)), sin(glm::radians(angle)), -sin(glm::radians(angle)), cos(glm::radians(angle)));
                    uvs.emplace_back(((glm::vec2(texture_block.corners[0], 1.0f - texture_block.corners[1]) - originTransform) * uvRotationTransform) + originTransform);
                    uvs.emplace_back(((glm::vec2(texture_block.corners[2], 1.0f - texture_block.corners[3]) - originTransform) * uvRotationTransform) + originTransform);
                    uvs.emplace_back(((glm::vec2(texture_block.corners[4], 1.0f - texture_block.corners[5]) - originTransform) * uvRotationTransform) + originTransform);
                    uvs.emplace_back(((glm::vec2(texture_block.corners[0], 1.0f - texture_block.corners[1]) - originTransform) * uvRotationTransform) + originTransform);
                    uvs.emplace_back(((glm::vec2(texture_block.corners[4], 1.0f - texture_block.corners[5]) - originTransform) * uvRotationTransform) + originTransform);
                    uvs.emplace_back(((glm::vec2(texture_block.corners[6], 1.0f - texture_block.corners[7]) - originTransform) * uvRotationTransform) + originTransform);
                    for (auto &uv : uvs) {
                        if (horizontalFlip) {
                            uv.x = 1.0f - uv.x;
                        }
                        if (verticalFlip) {
                            uv.y = 1.0f - uv.y;
                        }
                        uv.x *= gl_texture.max_u;
                        uv.y *= gl_texture.max_v;
                    }
                }
                    break;
                case CAR:
                    break;
            }
            break;
        case NFS_5:
            break;
        case UNKNOWN:
            break;
    }

    return uvs;
}

GLuint MakeTextureArray(std::map<unsigned int, Texture> &textures, bool repeatable) {
    ASSERT(textures.size() < MAX_TEXTURE_ARRAY_SIZE, "Configured maximum texture array size of " << MAX_TEXTURE_ARRAY_SIZE << " has been exceeded.");

    size_t max_width =0, max_height = 0;
    GLuint texture_name;

    glGenTextures(1, &texture_name);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_name);

    // Find the maximum width and height, so we can avoid overestimating with blanket values (256x256) and thereby scale UV's uneccesarily
    for(auto &texture: textures){
        if(texture.second.width > max_width) max_width = texture.second.width;
        if(texture.second.height > max_height) max_height = texture.second.height;
    }

    std::vector<uint32_t> clear_data(max_width * max_height, 0);

    LOG(INFO) << "Creating texture array with " << (int) textures.size() << " textures, max texture width " << max_width << ", max texture height " << max_height;
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 3, GL_RGBA8, static_cast<GLsizei>(max_width), static_cast<GLsizei>(max_height), MAX_TEXTURE_ARRAY_SIZE); // I should really call this on textures.size(), but the layer numbers are not linear up to textures.size(). HS Bloats tex index up over 2048.

    for (auto &texture : textures) {
        ASSERT(texture.second.width <= max_width, "Texture " << texture.second.id << " exceeds maximum specified texture size (" << max_width << ") for Array");
        ASSERT(texture.second.height <= max_height, "Texture " << texture.second.id << " exceeds maximum specified texture size (" << max_height << ") for Array");
        // Set the whole texture to transparent (so min/mag filters don't find bad data off the edge of the actual image data)
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, hsStockTextureIndexRemap(texture.first), static_cast<GLsizei>(max_width), static_cast<GLsizei>(max_height), 1, GL_RGBA, GL_UNSIGNED_BYTE, &clear_data[0]);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, hsStockTextureIndexRemap(texture.first), texture.second.width, texture.second.height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid *) texture.second.data);

        texture.second.min_u = 0.00;
        texture.second.min_v = 0.00;
        texture.second.layer = hsStockTextureIndexRemap(texture.first);
        texture.second.max_u = (texture.second.width / static_cast<float>(max_width )) - 0.005f; // Attempt to remove potential for sampling texture from transparent area
        texture.second.max_v = (texture.second.height / static_cast<float>(max_height)) - 0.005f;
        texture.second.id = texture_name;

    }

    if (repeatable) {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    //Unbind texture
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    return texture_name;
}

Texture LoadTexture(TexBlock trackTexture, const std::string &trackName)
{
    std::stringstream filename;
    std::stringstream filename_alpha;

    if (trackTexture.isLane)
    {
        filename << "../resources/sfx/" << std::setfill('0') << std::setw(4) << trackTexture.texture + 9 << ".BMP";
        filename_alpha << "../resources/sfx/" << std::setfill('0') << std::setw(4) << trackTexture.texture + 9 << "-a.BMP";
    } else
    {
        filename << TRACK_PATH << ToString(NFS_3) << "/" << trackName << "/textures/" << std::setfill('0') << std::setw(4)
                 << trackTexture.texture << ".BMP";
        filename_alpha << TRACK_PATH << ToString(NFS_3) << "/" << trackName << "/textures/" << std::setfill('0') << std::setw(4)
                       << trackTexture.texture << "-a.BMP";
    }

    GLubyte *data;
    GLsizei width = trackTexture.width;
    GLsizei height = trackTexture.height;

    if (!ImageLoader::LoadBmpWithAlpha(filename.str().c_str(), filename_alpha.str().c_str(), &data, &width, &height))
    {
        LOG(WARNING) << "Texture " << filename.str() << " or " << filename_alpha.str() << " did not load succesfully!";
        // If the texture is missing, load a "MISSING" texture of identical size.
        ASSERT(ImageLoader::LoadBmpWithAlpha("../resources/misc/missing.bmp", "../resources/misc/missing-a.bmp", &data, &width, &height), "Even the 'missing' texture is missing!");
        return Texture((unsigned int) trackTexture.texture, data, static_cast<unsigned int>(width), static_cast<unsigned int>(height));
    }

    return Texture((unsigned int) trackTexture.texture, data, static_cast<unsigned int>(trackTexture.width), static_cast<unsigned int>(trackTexture.height));
}