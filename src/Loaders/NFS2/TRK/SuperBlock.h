#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"
#include "TrackBlock.h"

namespace LibOpenNFS {
    namespace NFS2 {
        template <typename Platform>
        class SuperBlock : IRawData {
        public:
            SuperBlock() = default;
            explicit SuperBlock(std::ifstream &trk, NFSVersion version);
            void _SerializeOut(std::ofstream &ofstream) override;

            // ONFS attribute
            NFSVersion version;

            // Raw file data
            uint32_t superBlockSize;
            uint32_t nBlocks;
            uint32_t padding;
            std::vector<TrackBlock<Platform>> trackBlocks;
            std::vector<uint32_t> blockOffsets;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
        };
    } // namespace NFS2
} // namespace LibOpenNFS
