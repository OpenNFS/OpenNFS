#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"
#include "../TRK/ExtraObjectBlock.h"

namespace LibOpenNFS
{
    namespace NFS2
    {
        template <typename Platform>
        class ColFile : IRawData
        {
        public:
            ColFile() = default;
            static bool Load(const std::string &colPath, ColFile &colFile);
            static void Save(const std::string &colPath, ColFile &colFile);

            static const uint8_t HEADER_LENGTH = 4;

            unsigned char header[HEADER_LENGTH];
            uint32_t version;
            uint32_t size;
            uint32_t nExtraBlocks;
            std::vector<uint32_t> extraBlockOffsets;
            std::vector<ExtraObjectBlock<Platform>> extraObjectBlocks;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
            void _SerializeOut(std::ofstream &ofstream) override;
        };

    } // namespace NFS2
} // namespace LibOpenNFS
