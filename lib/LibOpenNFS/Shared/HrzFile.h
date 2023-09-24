#pragma once

#include "../Common/IRawData.h"

namespace LibOpenNFS {
    class HrzFile : IRawData {
    public:
        HrzFile() = default;
        static bool Load(const std::string &hrzPath, HrzFile &hrzFile);
        static void Save(const std::string &hrzPath, HrzFile &hrzFile);

        glm::vec3 skyTopColour;
        glm::vec3 skyBottomColour;

    private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS
