#pragma once

#include <map>

#include "../../Common/IRawData.h"
#include "../Common.h"
#include "../TRK/ExtraObjectBlock.h"

namespace LibOpenNFS {
    namespace NFS2 {
        template <typename Platform>
        class ColFile : IRawData {
        public:
            ColFile() = default;
            static bool Load(const std::string &colPath, ColFile &colFile, NFSVersion version);
            static void Save(const std::string &colPath, ColFile &colFile);
            ExtraObjectBlock<Platform> GetExtraObjectBlock(ExtraBlockID eBlockType);
            bool IsBlockPresent(ExtraBlockID eBlockType);

            static const uint8_t HEADER_LENGTH = 4;

            // ONFS attribute
            NFSVersion version;

            // Raw File data
            unsigned char header[HEADER_LENGTH];
            uint32_t colVersion;
            uint32_t size;
            uint32_t nExtraBlocks;
            std::vector<uint32_t> extraBlockOffsets;
            std::vector<ExtraObjectBlock<Platform>> extraObjectBlocks;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
            void _SerializeOut(std::ofstream &ofstream) override;

            // Allows lookup by block type for parsers
            std::map<ExtraBlockID, uint8_t> extraObjectBlockMap;
        };

    } // namespace NFS2
} // namespace LibOpenNFS
