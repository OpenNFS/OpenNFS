//
// Created by Amrik Sadhra on 19/07/2018.
//

#pragma once

#include "../Config.h"
#include "../Scene/Light.h"
#include "../Util/Utils.h"
#include "../nfs_data.h"
#include "bmpread.h"
#include <GL/glew.h>
#include <bitset>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

namespace TrackUtils {
Light MakeLight(glm::vec3 light_position, uint32_t light_type);

bool ExtractTrackTextures(const std::string &track_path, const ::std::string track_name, NFSVer nfs_version);

int hsStockTextureIndexRemap(int textureIndex);

GLuint MakeTextureArray(std::map<unsigned int, Texture> &textures, bool repeatable);

std::vector<glm::vec2> nfsUvGenerate(NFSVer tag, EntityType mesh_type, uint32_t textureFlags, Texture gl_texture);

std::vector<glm::vec2> nfsUvGenerate(NFSVer tag, EntityType mesh_type, uint32_t textureFlags, Texture gl_texture,
                                     NFS3_4_DATA::TEXTUREBLOCK texture_block);

bmpread_t RemapNFS2CarColours(bmpread_t bmpAttr);

bool LoadCAN(std::string can_path, std::vector<SHARED::CANPT> &cameraAnimations);

glm::vec3 parseRGBString(const std::string &rgb_string);

glm::vec3 calculateQuadNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4);

glm::vec3 calculateNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);

// Easily convert propietary and platform specific Vertices to glm::vec3. NFS2_DATA::PC::GEO::BLOCK_3D,
// NFS2_DATA::PS1::GEO::BLOCK_3D, NFS3_4_DATA::FLOATPT etc.
template <class NFSVertexStruct> glm::vec3 pointToVec(NFSVertexStruct block_3d) {
    return glm::vec3(block_3d.x, block_3d.y, block_3d.z);
}
}; // namespace TrackUtils
