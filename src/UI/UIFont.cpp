#include "UIFont.h"

#include <fstream>
#include <g3log/g3log.hpp>
#include <json/json.hpp>

namespace OpenNFS {
    std::map<std::string, UIFont> UIFont::LoadFonts(std::string const &fontFile) {
        using json = nlohmann::json;

        // Read the font JSON file
        std::ifstream jsonFile(fontFile);
        CHECK_F(jsonFile.is_open(), "Couldn't open font resource file %s", fontFile.c_str());

        std::map<std::string, UIFont> fonts;
        json fontsJson;
        jsonFile >> fontsJson;

        for (auto &el : fontsJson["fonts"].items()) {
            std::string fontName = el.value()["name"];
            std::string fontPath = el.value()["path"];
            int fontSize = el.value().value("size", 48); // Default size 48

            UIFont font{fontName, fontPath, fontSize};
            fonts.insert(std::pair(fontName, font));
            LOG(INFO) << "Loaded font: " << fontName << " from " << fontPath << " (size: " << fontSize << ")";
        }
        jsonFile.close();

        return fonts;
    }
} // namespace OpenNFS
