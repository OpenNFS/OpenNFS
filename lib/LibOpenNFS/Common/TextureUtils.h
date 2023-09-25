#pragma once

#include <string>
#include <filesystem>

#include "NFSVersion.h"
#include "Utils.h"

extern "C" {
#include "lib/fshtool/fshtool.h"
}

namespace LibOpenNFS {
    class TextureUtils {
    public:
        static uint32_t abgr1555ToARGB8888(uint16_t abgr_1555);
        static glm::vec3 HSLToRGB(glm::vec4 hsl);
        static glm::vec3 ParseRGBString(const std::string &rgb_string);
        // Break Packed uint32_t RGBA per vertex colour data for baked lighting of RGB into 4 normalised floats and store into vec4
        static glm::vec4 ShadingDataToVec4(uint32_t packed_rgba);
        static bool ExtractQFS(const std::string &qfs_input, const std::string &output_dir);
        static bool ExtractPSH(const std::string &psh_path, const std::string &output_path);
        bool ExtractTrackTextures(const std::string &trackPath, const ::std::string &trackName, NFSVersion nfsVer, const std::string &outPath);
        int32_t hsStockTextureIndexRemap(int32_t textureIndex);
    };

} // namespace LibOpenNFS