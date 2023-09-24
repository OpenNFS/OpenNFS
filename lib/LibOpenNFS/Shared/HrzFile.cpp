#include "HrzFile.h"

#include "Common/TextureUtils.h"

namespace LibOpenNFS {
    bool HrzFile::Load(const std::string &hrzPath, HrzFile &hrzFile) {
        // LOG(INFO) << "Loading HRZ File located at " << hrzPath;
        std::ifstream hrz(hrzPath, std::ios::in | std::ios::binary);

        bool loadStatus = hrzFile._SerializeIn(hrz);
        hrz.close();

        return loadStatus;
    }

    void HrzFile::Save(const std::string &hrzPath, HrzFile &hrzFile) {
        // LOG(INFO) << "Saving HRZ File to " << hrzPath;
        std::ofstream hrz(hrzPath, std::ios::out | std::ios::binary);
        hrzFile._SerializeOut(hrz);
    }

    bool HrzFile::_SerializeIn(std::ifstream &ifstream) {
        bool foundSkyTop    = false;
        bool foundSkyBottom = false;

        std::string str, strSkyTopColour, strSkyBottomColour;

        while (std::getline(ifstream, str)) {
            if (str.find("/* r,g,b value at top of Gourad shaded SKY area */") != std::string::npos) {
                std::getline(ifstream, strSkyTopColour);
                foundSkyTop  = true;
                skyTopColour = TextureUtils::ParseRGBString(strSkyTopColour);
            }
            if (str.find("/* r,g,b values for base of Gourad shaded SKY area */") != std::string::npos) {
                std::getline(ifstream, strSkyBottomColour);
                foundSkyBottom  = true;
                skyBottomColour = TextureUtils::ParseRGBString(strSkyBottomColour);
            }
        }

        return foundSkyTop && foundSkyBottom;
    }

    void HrzFile::_SerializeOut(std::ofstream &ofstream) {
        ASSERT(false, "HRZ Output serialization is not implemented yet");
    }
} // namespace LibOpenNFS
