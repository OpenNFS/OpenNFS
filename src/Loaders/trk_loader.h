//
// Created by Amrik.Sadhra on 20/06/2018.
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
#include "../Util/Utils.h"

std::map<short, GLuint> GenTrackTextures(std::map<short, Texture> textures);

std::vector<short> RemapTextureIDs(const std::set<short> &minimal_texture_ids_set, std::vector<unsigned int> &texture_indices) ;

bool ExtractTrackTextures(const std::string &track_path, const::std::string track_name, NFSVer nfs_version);
