//
// Created by Amrik on 29/03/2019.
//

#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint ImageLoader::LoadImage(const std::string &imagePath, int *width, int *height, GLint wrapParam, GLint sampleParam){
    GLuint textureID;
    int compressionType;

    unsigned char* image = stbi_load(imagePath.c_str(), width, height, &compressionType, STBI_rgb_alpha);
    ASSERT(image != nullptr, "Failed to load UI texture " << imagePath);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampleParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampleParam);

    switch(compressionType){
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, *width, *height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
            break;
        default:
            ASSERT(false, "Currently unsupported channel number/compression type (" << compressionType << ") in source image " << imagePath);
            break;
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);

    return textureID;
}