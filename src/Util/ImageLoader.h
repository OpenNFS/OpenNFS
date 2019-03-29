//
// Created by Amrik on 29/03/2019.
//

#pragma once

#include <GL/glew.h>
#include <string>

#include "Logger.h"

class ImageLoader {
public:
    explicit ImageLoader();
    ~ImageLoader();
    static GLuint LoadImage(const std::string &imagePath, int *width, int *height, GLint wrapParam, GLint sampleParam);
};
