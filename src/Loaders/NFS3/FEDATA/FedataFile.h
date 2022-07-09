#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS {
    namespace NFS3 {
        static const uint32_t COLOUR_TABLE_OFFSET      = 0xA7;
        static const uint32_t MENU_NAME_FILEPOS_OFFSET = 0x37;

        class FedataFile : IRawData {
        public:
            FedataFile() = default;

            static bool Load(const std::string &fedataPath, FedataFile &fedataFile, uint8_t nPriColours);
            static void Save(const std::string &fedataPath, FedataFile &fedataFile);

            std::string menuName;
            std::vector<std::string> primaryColourNames;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
            void _SerializeOut(std::ofstream &ofstream) override;

            uint8_t m_nPriColours;
        };
    } // namespace NFS3
} // namespace LibOpenNFS
