#pragma once

#include <map>

#include "../../Common/IRawData.h"
#include "../Common.h"
#include "ExtraObjectBlock.h"

namespace LibOpenNFS {
    namespace NFS2 {
        template <typename Platform>
        class TrackBlock : IRawData {
        public:
            TrackBlock() = default;
            explicit TrackBlock(std::ifstream &trk, NFSVersion version);
            void _SerializeOut(std::ofstream &ofstream) override;
            ExtraObjectBlock<Platform> GetExtraObjectBlock(ExtraBlockID eBlockType);
            bool IsBlockPresent(ExtraBlockID eBlockType);

            // ONFS attribute
            NFSVersion version;

            // Raw file data
            uint32_t blockSize;
            uint32_t blockSizeDup;
            uint16_t nExtraBlocks;
            uint16_t unknown;
            uint32_t serialNum;
            struct VERT_HIGHP clippingRect[4];
            uint32_t extraBlockTblOffset;
            uint16_t nStickToNextVerts, nLowResVert, nMedResVert, nHighResVert;
            uint16_t nLowResPoly, nMedResPoly, nHighResPoly;
            uint16_t unknownPad[3];
            std::vector<typename Platform::VERT> vertexTable;
            std::vector<typename Platform::POLYGONDATA> polygonTable;
            std::vector<uint32_t> extraBlockOffsets;
            std::vector<ExtraObjectBlock<Platform>> extraObjectBlocks;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;

            // Allows lookup by block type for parsers
            std::map<ExtraBlockID, uint8_t> extraObjectBlockMap;
        };
    } // namespace NFS2
} // namespace LibOpenNFS
