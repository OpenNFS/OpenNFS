#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS {
    namespace NFS3 {
        class SpeedsFile : IRawData {
        public:
            SpeedsFile() = default;
            static bool Load(const std::string &speedBinPath, SpeedsFile &speedFile);
            static void Save(const std::string &speedBinPath, SpeedsFile &speedFile);
            static void SaveCSV(const std::string &speedsCsvPath, SpeedsFile &speedFile);

            std::vector<uint8_t> speeds;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
            void _SerializeOut(std::ofstream &ofstream) override;

            uint16_t m_uFileSize = 0;
        };
    } // namespace NFS3
} // namespace LibOpenNFS
