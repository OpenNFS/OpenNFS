#pragma once

#include "../../Common/IRawData.h"

#include "SuperBlock.h"

namespace LibOpenNFS {
    namespace NFS2 {
        template <typename Platform>
        class TrkFile : IRawData {
        public:
            TrkFile() = default;
            static bool Load(const std::string &trkPath, TrkFile &trkFile, NFSVersion version);
            static void Save(const std::string &trkPath, TrkFile &trkFile);

            static const uint8_t HEADER_LENGTH         = 4;
            static const uint8_t UNKNOWN_HEADER_LENGTH = 5;

            // ONFS attribute
            NFSVersion version;

            // Raw File data
            unsigned char header[HEADER_LENGTH];
            uint32_t unknownHeader[UNKNOWN_HEADER_LENGTH];
            uint32_t nSuperBlocks;
            uint32_t nBlocks;
            std::vector<SuperBlock<Platform>> superBlocks;
            std::vector<uint32_t> superBlockOffsets;
            std::vector<VERT_HIGHP> blockReferenceCoords;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
            void _SerializeOut(std::ofstream &ofstream) override;
        };
    } // namespace NFS2
} // namespace LibOpenNFS
