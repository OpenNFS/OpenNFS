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

class Texture
{
public:
	Texture() = default;
	explicit Texture(NFSVer tag, uint32_t id, GLubyte *data, uint32_t width, uint32_t height, RawTextureInfo rawTextureInfo);
	std::vector<glm::vec2> GenerateUVs(EntityType meshType, uint32_t textureFlags, TexBlock texBlock);

	// Utils
	static Texture LoadTexture(NFSVer tag, TexBlock trackTexture, const std::string &trackName);
	static bool ExtractTrackTextures(const std::string &trackPath, const ::std::string trackName, NFSVer nfsVer);
	static int32_t hsStockTextureIndexRemap(int32_t textureIndex);
	static GLuint MakeTextureArray(std::map<uint32_t, Texture> &textures, bool repeatable);

	NFSVer tag;
	uint32_t id, width, height, layer;
	float min_u, min_v, max_u, max_v;
	GLubyte *data;
	RawTextureInfo rawTextureInfo;
};