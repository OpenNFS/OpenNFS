#include "GLTexture.h"

#include <utility>

GLTexture::GLTexture(LibOpenNFS::Texture texture, GLubyte *data) : texture(std::move(texture)), data(data) {
}

GLTexture GLTexture::LoadTexture(NFSVersion tag, const LibOpenNFS::Texture &rawTrackTexture, const std::string &trackName) {
    std::stringstream filename;
    GLubyte *data;
    GLsizei width;
    GLsizei height;

    switch (tag) {
    case NFSVersion::NFS_3: {
        width  = rawTrackTexture.GetWidth();
        height = rawTrackTexture.GetHeight();

        std::stringstream filename_alpha;

        if (rawTrackTexture.IsLane()) {
            filename << "../resources/sfx/" << std::setfill('0') << std::setw(4) << rawTrackTexture.GetTextureID() + 9 << ".BMP";
            filename_alpha << "../resources/sfx/" << std::setfill('0') << std::setw(4) << rawTrackTexture.GetTextureID() + 9 << "-a.BMP";
        } else {
            filename << TRACK_PATH << get_string(NFSVersion::NFS_3) << "/" << trackName << "/textures/" << std::setfill('0') << std::setw(4) << rawTrackTexture.GetTextureID()
                     << ".BMP";
            filename_alpha << TRACK_PATH << get_string(NFSVersion::NFS_3) << "/" << trackName << "/textures/" << std::setfill('0') << std::setw(4) << rawTrackTexture.GetTextureID()
                           << "-a.BMP";
        }

        if (!ImageLoader::LoadBmpWithAlpha(filename.str().c_str(), filename_alpha.str().c_str(), &data, &width, &height)) {
            LOG(WARNING) << "Texture " << filename.str() << " or " << filename_alpha.str() << " did not load succesfully!";
            // If the texture is missing, load a "MISSING" texture of identical size.
            ASSERT(ImageLoader::LoadBmpWithAlpha("../resources/misc/missing.bmp", "../resources/misc/missing-a.bmp", &data, &width, &height),
                   "Even the 'missing' texture is missing!");
            // TODO: Override width and height to missing resource attributes
            return GLTexture(rawTrackTexture, data);
        }

        return GLTexture(rawTrackTexture, data);
    }
    case NFSVersion::NFS_2:
    case NFSVersion::NFS_2_SE:
    case NFSVersion::NFS_2_PS1:
    case NFSVersion::NFS_3_PS1: {
        uint8_t alphaColour = 0;
        switch (tag) {
        case NFSVersion::NFS_2:
            alphaColour = 0u;
            break;
        case NFSVersion::NFS_2_SE:
            alphaColour = 248u;
            break;
        default:
            assert(false);
        }
        filename << TRACK_PATH << get_string(tag) << "/" << trackName << "/textures/" << std::setfill('0') << std::setw(4) << rawTrackTexture.GetTextureID() << ".BMP";

        ASSERT(ImageLoader::LoadBmpCustomAlpha(filename.str().c_str(), &data, &width, &height, alphaColour), "Texture " << filename.str() << " did not load succesfully!");

        return GLTexture(rawTrackTexture, data);
    }
    default:
        ASSERT(false, "Trying to load texture from unknown NFS version");
    }
}

GLuint GLTexture::MakeTextureArray(std::map<uint32_t, GLTexture> &textures, bool repeatable) {
    ASSERT(textures.size() < MAX_TEXTURE_ARRAY_SIZE, "Configured maximum texture array size of " << MAX_TEXTURE_ARRAY_SIZE << " has been exceeded");

    size_t max_width = 0, max_height = 0;
    GLuint texture_name;

    glGenTextures(1, &texture_name);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_name);

    // Find the maximum width and height, so we can avoid overestimating with blanket values (256x256) and thereby scale UV's uneccesarily
    for (auto &texture : textures) {
        if (texture.second.width > max_width)
            max_width = texture.second.width;
        if (texture.second.height > max_height)
            max_height = texture.second.height;
    }

    std::vector<uint32_t> clear_data(max_width * max_height, 0);

    LOG(INFO) << "Creating texture array with " << (int) textures.size() << " textures, max texture width " << max_width << ", max texture height " << max_height;
    glTexStorage3D(GL_TEXTURE_2D_ARRAY,
                   3,
                   GL_RGBA8,
                   static_cast<GLsizei>(max_width),
                   static_cast<GLsizei>(max_height),
                   MAX_TEXTURE_ARRAY_SIZE); // I should really call this on textures.size(), but the layer numbers are not linear up to
                                            // textures.size(). HS Bloats tex index up over 2048.

    for (auto &texture : textures) {
        ASSERT(texture.second.width <= max_width, "Texture " << texture.second.id << " exceeds maximum specified texture size (" << max_width << ") for Array");
        ASSERT(texture.second.height <= max_height, "Texture " << texture.second.id << " exceeds maximum specified texture size (" << max_height << ") for Array");
        // Set the whole texture to transparent (so min/mag filters don't find bad data off the edge of the actual image data)
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                        0,
                        0,
                        0,
                        hsStockTextureIndexRemap(texture.first),
                        static_cast<GLsizei>(max_width),
                        static_cast<GLsizei>(max_height),
                        1,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        &clear_data[0]);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                        0,
                        0,
                        0,
                        hsStockTextureIndexRemap(texture.first),
                        texture.second.width,
                        texture.second.height,
                        1,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        (const GLvoid *) texture.second.data);

        texture.second.minU  = 0.00;
        texture.second.minV  = 0.00;
        texture.second.layer = hsStockTextureIndexRemap(texture.first);
        texture.second.maxU  = (texture.second.width / static_cast<float>(max_width)) - 0.005f; // Attempt to remove potential for sampling texture from transparent area
        texture.second.maxV  = (texture.second.height / static_cast<float>(max_height)) - 0.005f;
        texture.second.id    = texture_name;
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

int32_t GLTexture::hsStockTextureIndexRemap(int32_t textureIndex) {
        int32_t remappedIndex = textureIndex;

        int32_t nStockTextures = 30;

        // Remap texture index between 0 and MAX_TEXTURE_ARRAY_SIZE if exceeds
        if (textureIndex >= 2048) {
            remappedIndex = MAX_TEXTURE_ARRAY_SIZE - nStockTextures + (textureIndex - 2048);
        }

        ASSERT(remappedIndex <= MAX_TEXTURE_ARRAY_SIZE, "Texture index still exceeds max texture array size, post-remap");

        return remappedIndex;
    }