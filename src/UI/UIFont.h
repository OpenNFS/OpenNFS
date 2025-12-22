#pragma once

#include <map>
#include <string>

namespace OpenNFS {
    struct UIFont {
        std::string name;
        std::string path;
        int size;

        static std::map<std::string, UIFont> LoadFonts(std::string const &fontFile);
    };
} // namespace OpenNFS
