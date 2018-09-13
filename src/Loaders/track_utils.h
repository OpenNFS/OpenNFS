//
// Created by Amrik Sadhra on 19/07/2018.
//

#pragma once

#include <string>
#include <set>
#include <sstream>
#include <iostream>
#include <bitset>
#include <GL/glew.h>
#include <boost/filesystem/operations.hpp>
#include "../nfs_data.h"
#include "../Config.h"
#include "../Scene/Light.h"
#include "../Util/Utils.h"

namespace TrackUtils {
    Light MakeLight(glm::vec3 light_position, uint32_t light_type);

    bool ExtractTrackTextures(const std::string &track_path, const::std::string track_name, NFSVer nfs_version);

    int hsStockTextureIndexRemap(int textureIndex);

    GLuint MakeTextureArray(std::map<unsigned int, Texture> &textures, size_t max_width, size_t max_height, bool repeatable);

    std::vector<glm::vec2> nfsUvGenerate(NFSVer tag, EntityType mesh_type, uint32_t textureFlags, Texture gl_texture);

    std::vector<glm::vec2> nfsUvGenerate(NFSVer tag, EntityType mesh_type, uint32_t textureFlags, Texture gl_texture, NFS3_4_DATA::TEXTUREBLOCK texture_block);

    bool LoadCAN(std::string can_path, std::vector<SHARED::CANPT> &cameraAnimations);

    glm::vec3 parseRGBString(const std::string &rgb_string);
};



