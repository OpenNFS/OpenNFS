#include "GLTexture.h"

#include "../Util/Utils.h"
#include <utility>

namespace OpenNFS {
    GLTexture::GLTexture(LibOpenNFS::TrackTextureAsset texture, std::vector<uint8_t> &data)
        : texture_asset(std::move(texture)), data(data) {
    }

    GLubyte *GLTexture::GetData() {
        return data.data();
    }

    GLTexture GLTexture::LoadTexture(NFSVersion const tag, LibOpenNFS::TrackTextureAsset &trackTextureAsset) {
        GLsizei width;
        GLsizei height;
        std::vector<uint8_t> data;

        switch (tag) {
        case NFSVersion::NFS_3: {
            if (!ImageLoader::LoadBmpWithAlpha(trackTextureAsset.fileReference.c_str(), trackTextureAsset.alphaFileReference.c_str(), data,
                                               &width, &height)) {
                LOG(WARNING) << "Texture " << trackTextureAsset.fileReference << " or " << trackTextureAsset.alphaFileReference
                             << " did not load succesfully!";
                // If the texture is missing, load a "MISSING" texture of identical size.
                CHECK_F(ImageLoader::LoadBmpWithAlpha("../resources/misc/missing.bmp", "../resources/misc/missing-a.bmp", data, &width,
                                                      &height),
                        "Even the 'missing' texture is missing!");
                // Override texture with missing texture attributes
                trackTextureAsset.width = width;
                trackTextureAsset.height = height;
            }

            return GLTexture(trackTextureAsset, data);
        }
        case NFSVersion::NFS_2:
        case NFSVersion::NFS_2_SE:
        case NFSVersion::NFS_2_PS1:
        case NFSVersion::NFS_3_PS1: {
            uint8_t const alphaColour = (tag == NFSVersion::NFS_2_SE) ? 248u : 0;
            CHECK_F(ImageLoader::LoadBmpCustomAlpha(trackTextureAsset.fileReference.c_str(), data, &width, &height, alphaColour),
                    "Texture %s did not load successfully!", trackTextureAsset.fileReference.c_str());
            // Write texture asset metadata as NFS2 doesn't populate them directly
            trackTextureAsset.width = width;
            trackTextureAsset.height = height;

            return GLTexture(trackTextureAsset, data);
        }
        case NFSVersion::NFS_4:
            if (!ImageLoader::LoadBmpCustomAlpha(trackTextureAsset.fileReference.c_str(), data, &width, &height, 0)) {
                LOG(WARNING) << "Texture " << trackTextureAsset.fileReference << " or " << trackTextureAsset.alphaFileReference
                             << " did not load succesfully!";
                // If the texture is missing, load a "MISSING" texture of identical size.
                CHECK_F(ImageLoader::LoadBmpWithAlpha("../resources/misc/missing.bmp", "../resources/misc/missing-a.bmp", data, &width,
                                                      &height),
                        "Even the 'missing' texture is missing!");
            }
            // NFS4 doesn't populate width and height from geometry files
            trackTextureAsset.width = width;
            trackTextureAsset.height = height;
            return GLTexture(trackTextureAsset, data);
        default:
            CHECK_F(false, "Trying to load texture from unknown NFS version");
        }
        unreachable();
    }

    GLuint GLTexture::MakeTextureArray(std::map<uint32_t, GLTexture> &textures, bool const repeatable) {
        CHECK_F(textures.size() < MAX_TEXTURE_ARRAY_SIZE, "Configured maximum texture array size of %d has been exceeded",
                MAX_TEXTURE_ARRAY_SIZE);

        size_t max_width = 0, max_height = 0;
        GLuint texture_name;

        glGenTextures(1, &texture_name);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture_name);

        // Find the maximum width and height, so we can avoid overestimating with blanket values (256x256) and thereby scale UV's
        // uneccesarily
        for (auto &[id, glTexture] : textures) {
            if (glTexture.texture_asset.width > max_width)
                max_width = glTexture.texture_asset.width;
            if (glTexture.texture_asset.height > max_height)
                max_height = glTexture.texture_asset.height;
        }

        std::vector<uint32_t> const clear_data(max_width * max_height, 0);

        LOG(INFO) << "Creating texture array with " << (int)textures.size() << " textures, max texture width " << max_width
                  << ", max texture height " << max_height;
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 3, GL_RGBA8, static_cast<GLsizei>(max_width), static_cast<GLsizei>(max_height),
                       MAX_TEXTURE_ARRAY_SIZE);
        // I should really call this on textures.size(), but the layer numbers are not linear up to
        // textures.size(). HS Bloats tex index up over 2048.

        for (auto &[id, glTexture] : textures) {
            CHECK_F(glTexture.texture_asset.width <= max_width, "Texture %u exceeds maximum specified texture width (%zu) for Array",
                    glTexture.texture_asset.id, max_width);
            CHECK_F(glTexture.texture_asset.width <= max_width, "Texture %u exceeds maximum specified texture height (%zu) for Array",
                    glTexture.texture_asset.id, max_height);

            // Set the whole texture to transparent (so min/mag filters don't find bad data off the edge of the actual image data)
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, hsStockTextureIndexRemap(id), static_cast<GLsizei>(max_width),
                            static_cast<GLsizei>(max_height), 1, GL_RGBA, GL_UNSIGNED_BYTE, &clear_data[0]);
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, hsStockTextureIndexRemap(id), glTexture.texture_asset.width,
                            glTexture.texture_asset.height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid const *)glTexture.GetData());

            glTexture.texture_asset.layer = hsStockTextureIndexRemap(id);
            glTexture.texture_asset.id = texture_name;
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

        // Unbind texture
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        return texture_name;
    }

    int32_t GLTexture::hsStockTextureIndexRemap(int32_t const textureIndex) {
        int32_t remappedIndex = textureIndex;

        // Remap texture index between 0 and MAX_TEXTURE_ARRAY_SIZE if exceeds
        if (textureIndex >= 2048) {
            constexpr int32_t nStockTextures{30};
            remappedIndex = MAX_TEXTURE_ARRAY_SIZE - nStockTextures + (textureIndex - 2048);
        }

        CHECK_F(remappedIndex <= MAX_TEXTURE_ARRAY_SIZE, "Texture index still exceeds max texture array size, post-remap");

        return remappedIndex;
    }
} // namespace OpenNFS
