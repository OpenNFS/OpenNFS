#pragma once

#include <GL/glew.h>
#include <string>
#include <set>
#include <sstream>
#include <iostream>
#include <bitset>
#include <boost/variant.hpp>
#include <boost/filesystem/operations.hpp>

#include "../Loaders/NFS3/FRD/TexBlock.h"
#include "../nfs_data.h"
#include "../Util/Utils.h"
#include "../Util/ImageLoader.h"

typedef boost::variant<TexBlock, NFS2_DATA::TEXTURE_BLOCK> RawTextureInfo;

class Texture {
public:
    Texture() = default;
    explicit Texture(uint32_t id, GLubyte *data, uint32_t width, uint32_t height);
    static bool ExtractTrackTextures(const std::string &track_path, const ::std::string track_name, NFSVer nfs_version);

    std::vector<glm::vec2> GenerateUVs(NFSVer tag, EntityType mesh_type, uint32_t textureFlags);
    static std::vector<glm::vec2> GenerateUVs(NFSVer tag, EntityType mesh_type, uint32_t textureFlags, Texture gl_texture, TexBlock texture_block);
    static GLuint MakeTextureArray(std::map<unsigned int, Texture> &textures, bool repeatable);

    uint32_t id, width, height, layer;
    float min_u, min_v, max_u, max_v;
    GLubyte *data;
    RawTextureInfo rawTextureInfo;
};