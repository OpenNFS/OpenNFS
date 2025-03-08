#pragma once

#include "../Util/ImageLoader.h"

#include <GL/glew.h>
#include <map>
#include <string>

namespace OpenNFS {
    class UIResource {
      public:
        static std::map<std::string, UIResource> LoadResources(std::string const &resourceFile);

        GLuint textureID;
        int width;
        int height;
    };
} // namespace OpenNFS
