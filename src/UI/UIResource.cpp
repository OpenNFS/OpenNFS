#include "UIResource.h"

#include <fstream>
#include <g3log/g3log.hpp>
#include <json/json.hpp>

namespace OpenNFS {
    std::map<std::string, UIResource> UIResource::LoadResources(std::string const &resourceFile) {
        using json = nlohmann::json;

        // Read the resource JSON file
        std::ifstream jsonFile(resourceFile);
        CHECK_F(jsonFile.is_open(), "Couldn't open menu resource file %s", resourceFile.c_str());

        std::map<std::string, UIResource> resources;
        json resourcesJson;
        jsonFile >> resourcesJson;

        for (auto &el : resourcesJson["resources"].items()) {
            std::string elementName = el.value()["name"];

            // Load the image into the GPU and get corresponding handle
            int width, height;
            GLuint const textureID{ImageLoader::LoadImage(el.value()["path"], &width, &height, GL_CLAMP_TO_EDGE, GL_LINEAR)};
            // Now store menu resource for later use
            UIResource menuResource{textureID, width, height};
            resources.insert(std::pair(elementName, menuResource));
        }
        jsonFile.close();

        return resources;
    }
}