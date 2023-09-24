#pragma once

#include "Common/IRawData.h"

namespace LibOpenNFS::Shared {
    struct VivEntry {
        char filename[100];
        std::vector<uint8_t> data;
    };

    class VivFile : IRawData {
    public:
        VivFile() = default;
        static bool Load(const std::string &vivPath, VivFile &vivFile);
        static void Save(const std::string &vivPath, VivFile &vivFile);
        static bool Extract(const std::string &outPath, VivFile &vivFile);

        char vivHeader[4];
        uint32_t vivSize;
        uint32_t nFiles;
        uint32_t startPos;
        std::vector<VivEntry> files;

    private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
        uint32_t _SwapEndian(uint32_t x);
    };
} // namespace LibOpenNFS::Shared