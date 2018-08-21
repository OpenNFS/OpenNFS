//
// Created by Amrik Sadhra on 19/07/2018.
//

#pragma once

#include <string>
#include <set>
#include <sstream>
#include <iostream>
#include <GL/glew.h>
#include <boost/filesystem/operations.hpp>
#include "../nfs_data.h"
#include "../Config.h"
#include "../Scene/Light.h"
#include "../Util/Utils.h"

namespace TrackUtils {
    std::map<short, GLuint> GenTrackTextures(std::map<short, Texture> textures);

    std::vector<unsigned int> RemapTextureIDs(const std::set<unsigned int> &minimal_texture_ids_set, std::vector<unsigned int> &texture_indices) ;

    Light MakeLight(glm::vec3 light_position, uint32_t light_type);

    bool ExtractTrackTextures(const std::string &track_path, const::std::string track_name, NFSVer nfs_version);
};



