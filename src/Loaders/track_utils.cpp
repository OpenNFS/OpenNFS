//
// Created by Amrik Sadhra on 19/07/2018.
//

#include "track_utils.h"

namespace TrackUtils {
    Light MakeLight(glm::vec3 light_position, uint32_t light_type) {
        // Use Data from NFSHS NFS3 Tracks TR.INI
        switch (light_type) {
            case 0:
                return Light(light_position, glm::vec4(255, 222, 234, 235), light_type, 0, 0, 0, 5.00f);
            case 1:
                return Light(light_position, glm::vec4(185, 255, 255, 255), light_type, 0, 0, 0, 4.50f);
            case 2:
                return Light(light_position, glm::vec4(255, 255, 255, 210), light_type, 0, 0, 0, 5.00f);
            case 3:
                return Light(light_position, glm::vec4(255, 128, 229, 240), light_type, 0, 0, 0, 4.50f);
            case 4:
                return Light(light_position, glm::vec4(255, 217, 196, 94), light_type, 0, 0, 0, 5.00f);
            case 5:
                return Light(light_position, glm::vec4(255, 223, 22, 22), light_type, 1, 6, 0, 5.00f);
            case 6:
                return Light(light_position, glm::vec4(255, 223, 22, 22), light_type, 1, 5, 27, 5.00f);
            case 7:
                return Light(light_position, glm::vec4(255, 255, 0, 0), light_type, 1, 6, 0, 3.13f);
            case 8:
                return Light(light_position, glm::vec4(255, 163, 177, 190), light_type, 0, 0, 0, 3.75f);
            case 9:
                return Light(light_position, glm::vec4(255, 223, 22, 22), light_type, 0, 0, 0, 3.13f);
            case 10:
                return Light(light_position, glm::vec4(186, 223, 22, 22), light_type, 0, 0, 0, 2.50f);
            case 11:
                return Light(light_position, glm::vec4(255, 30, 149, 227), light_type, 0, 0, 0, 2.50f);
            case 12:
                return Light(light_position, glm::vec4(255, 30, 149, 227), light_type, 1, 6, 0, 3.13f);
            case 13:
                return Light(light_position, glm::vec4(255, 224, 224, 39), light_type, 0, 0, 0, 3.75f);
            case 14:
                return Light(light_position, glm::vec4(255, 222, 234, 235), light_type, 0, 0, 0, 5.00f);
            case 15:
                return Light(light_position, glm::vec4(255, 222, 234, 235), light_type, 0, 0, 0, 5.00f);
            case 16:
                return Light(light_position, glm::vec4(255, 222, 234, 235), light_type, 0, 0, 0, 5.00f);
            case 17:
                return Light(light_position, glm::vec4(185, 255, 255, 255), light_type, 0, 0, 0, 4.50f);
            case 18:
                return Light(light_position, glm::vec4(255, 255, 255, 210), light_type, 0, 0, 0, 5.00f);
            case 19:
                return Light(light_position, glm::vec4(255, 128, 229, 240), light_type, 0, 0, 0, 4.50f);
            case 20:
                return Light(light_position, glm::vec4(255, 217, 196, 94), light_type, 0, 0, 0, 5.00f);
            case 21:
                return Light(light_position, glm::vec4(255, 223, 22, 22), light_type, 1, 6, 0, 5.00f);
            case 22:
                return Light(light_position, glm::vec4(255, 223, 22, 22), light_type, 1, 5, 27, 5.00f);
            case 23:
                return Light(light_position, glm::vec4(255, 255, 0, 0), light_type, 1, 6, 0, 3.13f);
            case 24:
                return Light(light_position, glm::vec4(255, 163, 177, 190), light_type, 0, 0, 0, 3.75f);
            case 25:
                return Light(light_position, glm::vec4(255, 223, 22, 22), light_type, 0, 0, 0, 3.13f);
            case 26:
                return Light(light_position, glm::vec4(186, 223, 22, 22), light_type, 0, 0, 0, 2.50f);
            case 27:
                return Light(light_position, glm::vec4(255, 30, 149, 227), light_type, 0, 0, 0, 2.50f);
            case 28:
                return Light(light_position, glm::vec4(255, 30, 149, 227), light_type, 1, 6, 0, 3.13f);
            case 29:
                return Light(light_position, glm::vec4(255, 224, 224, 39), light_type, 0, 0, 0, 3.75f);
            case 30:
                return Light(light_position, glm::vec4(255, 222, 234, 235), light_type, 0, 0, 0, 5.00f);
            case 31:
                return Light(light_position, glm::vec4(255, 222, 234, 235), light_type, 0, 0, 0, 5.00f);
            default:
                return Light(light_position, glm::vec4(255, 255, 255, 255), light_type, 0, 0, 0, 5.00f);
        }
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

        std::cout << "Extracting track textures" << std::endl;

        if (nfs_version == NFS_3_PS1) {
            output_dir << "/textures/";
            return Utils::ExtractPSH(tex_archive_path.str(), output_dir.str());
        } else if (nfs_version == NFS_3) {
            std::stringstream sky_fsh_path;
            sky_fsh_path << full_track_path.substr(0, full_track_path.find_last_of('/')) << "/sky.fsh";
            if (boost::filesystem::exists(sky_fsh_path.str())) {
                std::stringstream sky_textures_path;
                sky_textures_path << output_dir.str() << "/sky_textures/";
                std::cout << sky_fsh_path.str() << std::endl;
                ASSERT(Utils::ExtractQFS(sky_fsh_path.str(), sky_textures_path.str()), "Unable to extract sky textures from sky.fsh");
            }
        }

        output_dir << "/textures/";
        return (Utils::ExtractQFS(tex_archive_path.str(), output_dir.str()));
    }

    GLuint MakeTextureArray(std::map<unsigned int, Texture> &textures, size_t max_width, size_t max_height, bool repeatable) {
        GLuint texture_name;
        glGenTextures(1, &texture_name);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture_name);
        std::vector<uint32_t> clear_data(max_width * max_height, 0);

        std::cout << "Creating texture array with " << (int) textures.size() << " textures, max texture width " << max_width << ", max texture height " << max_height << std::endl;
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 3, GL_RGBA8, max_width, max_height, textures.size());

        uint32_t preMap = textures.size();
        for (uint32_t i = 0; i < textures.size(); i++) {
            auto it = textures.find(i);
            if (it != textures.end()) {
                // TODO: This will create a texture if it doesn't exist in the map. I should retrieve a texture in this case so that it doesn't look silly.
                auto &texture = textures[i];

                ASSERT(texture.width <= max_width, "Texture " << texture.texture_id << " exceeds maximum specified texture size (" << max_width << ") for Array");
                ASSERT(texture.height <= max_height, "Texture " << texture.texture_id << " exceeds maximum specified texture size (" << max_height << ") for Array");
                // Set the whole texture to transparent (so min/mag filters don't find bad data off the edge of the actual image data)
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, max_width, max_height, 1, GL_RGBA, GL_UNSIGNED_BYTE, &clear_data[0]);
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, texture.width, texture.height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid *) texture.texture_data);

                texture.min_u = 0.00;
                texture.min_v = 0.00;
                texture.layer = i;
                texture.max_u = (texture.width / static_cast<float>(max_width )) - 0.01f; // Attempt to remove potential for sampling texture from transparent area
                texture.max_v = (texture.height / static_cast<float>(max_height)) - 0.01f;
                texture.texture_id = texture_name;
            }
        }

        if (preMap != textures.size()) {
            std::cerr << "Texture array creation created " << textures.size() - preMap << " extra textures" << std::endl;
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

    std::vector<glm::vec2> nfsUvGenerate(NFSVer tag, EntityType mesh_type, uint32_t textureFlags, Texture gl_texture) {
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
                        bool verticalFlip =  false; //textureAlignment[9];
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
                            uv.x *= gl_texture.max_u;
                            uv.y *= gl_texture.max_v;
                        }
                    }
                        break;
                    case OBJ_POLY:
                        break;
                    case ROAD:
                        uvs.emplace_back(1.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
                        uvs.emplace_back(0.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
                        uvs.emplace_back(0.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
                        uvs.emplace_back(1.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
                        uvs.emplace_back(0.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
                        uvs.emplace_back(1.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
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
                        uvs.emplace_back(1.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
                        uvs.emplace_back(0.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
                        uvs.emplace_back(0.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
                        uvs.emplace_back(1.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
                        uvs.emplace_back(0.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
                        uvs.emplace_back(1.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
                        break;
                    case OBJ_POLY:
                        break;
                    case ROAD:
                        uvs.emplace_back(1.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
                        uvs.emplace_back(0.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
                        uvs.emplace_back(0.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
                        uvs.emplace_back(1.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
                        uvs.emplace_back(0.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
                        uvs.emplace_back(1.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
                        break;
                    case GLOBAL:
                        break;
                    case CAR: {
                        bool horizontalFlip = !textureAlignment[4];
                        bool verticalFlip = true;
                        glm::vec2 originTransform = glm::vec2(0.5f, 0.5f);
                        uint8_t nRotate = static_cast<uint8_t>((textureFlags >> 5) & 3);
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
                            uv.x *= gl_texture.max_u;
                            uv.y *= gl_texture.max_v;
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

    std::vector<glm::vec2> nfsUvGenerate(NFSVer tag, EntityType mesh_type, uint32_t textureFlags, Texture gl_texture, NFS3_4_DATA::TEXTUREBLOCK texture_block) {
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
                switch(mesh_type){
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
                    case GLOBAL:break;
                    case CAR:break;
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
                    case XOBJ:
                    {
                        bool horizontalFlip = textureAlignment[4];
                        bool verticalFlip   = textureAlignment[5];
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
                    case GLOBAL:
                    {
                        bool horizontalFlip = textureAlignment[4];
                        bool verticalFlip   = textureAlignment[5];
                        glm::vec2 originTransform = glm::vec2(0.5f, 0.5f);
                        uint8_t nRotate = static_cast<uint8_t>((textureFlags >> 2) & 3);
                        float angle = nRotate * 90.f;
                        glm::mat2 uvRotationTransform = glm::mat2(cos(glm::radians(angle)), sin(glm::radians(angle)), -sin(glm::radians(angle)), cos(glm::radians(angle)));
                        uvs.emplace_back(((glm::vec2(texture_block.corners[0], 1.0f- texture_block.corners[1]) - originTransform) * uvRotationTransform) + originTransform);
                        uvs.emplace_back(((glm::vec2(texture_block.corners[2], 1.0f- texture_block.corners[3]) - originTransform) * uvRotationTransform) + originTransform);
                        uvs.emplace_back(((glm::vec2(texture_block.corners[4], 1.0f- texture_block.corners[5]) - originTransform) * uvRotationTransform) + originTransform);
                        uvs.emplace_back(((glm::vec2(texture_block.corners[0], 1.0f- texture_block.corners[1]) - originTransform) * uvRotationTransform) + originTransform);
                        uvs.emplace_back(((glm::vec2(texture_block.corners[4], 1.0f- texture_block.corners[5]) - originTransform) * uvRotationTransform) + originTransform);
                        uvs.emplace_back(((glm::vec2(texture_block.corners[6], 1.0f- texture_block.corners[7]) - originTransform) * uvRotationTransform) + originTransform);
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

    bool LoadCAN(std::string can_path, std::vector<SHARED::CANPT> &cameraAnimations) {
        ifstream can(can_path, ios::in | ios::binary);

        if (!can.is_open()) {
            return false;
        }

        std::cout << "Loading CAN File" << std::endl;

        // Get filesize so can check have parsed all bytes
        can.seekg(0, ios_base::end);
        streamoff fileSize = can.tellg();
        can.seekg(0, ios_base::beg);

        /* uint16_t size;
        char type;     // 1 = basic object, 3 = animated ...
        char struct3D; // reference in previous block
// type 1
        INTPT ptRef;
// type 3
        uint16_t animLength;
        uint16_t unknown;*/
        uint8_t header[8];
        can.read((char *) header, sizeof(uint8_t) * 8);
        // 4th byte is the number of ANIMDATA points in the CAN file
        uint8_t nAnimations = header[4];

        SHARED::CANPT *anim = new SHARED::CANPT[nAnimations];
        can.read((char *) anim, sizeof(SHARED::CANPT) * nAnimations);

        for (uint8_t anim_Idx = 0; anim_Idx < nAnimations; ++anim_Idx) {
            cameraAnimations.emplace_back(anim[anim_Idx]);
        }

        streamoff readBytes = can.tellg();

        ASSERT(readBytes == fileSize, "Missing " << fileSize - readBytes << " bytes from loaded CAN file: " << can_path);

        return true;
    }

    glm::vec3 parseRGBString(const std::string &rgb_string) {
        std::stringstream tempComponent;
        uint8_t commaCount = 0;
        glm::vec3 rgbValue;

        for (int char_Idx = 0; char_Idx < rgb_string.length(); ++char_Idx) {
            if (rgb_string[char_Idx] == ',') {
                switch (commaCount) {
                    case 0:
                        rgbValue.x = (float) stoi(tempComponent.str());
                        break;
                    case 1:
                        rgbValue.y = (float) stoi(tempComponent.str());
                        break;
                    case 2:
                        rgbValue.z = (float) stoi(tempComponent.str());
                        break;
                }
                tempComponent.str("");
                if (++commaCount >= 3) break;
            } else {
                tempComponent << rgb_string[char_Idx];
            }
        }

        return rgbValue;
    }
}
