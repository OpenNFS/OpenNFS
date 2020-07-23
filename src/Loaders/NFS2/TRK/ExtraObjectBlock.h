#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"
#include "StructureBlock.h"

namespace LibOpenNFS
{
    namespace NFS2
    {
        // Matches number of NP1 polygons in corresponding trackblock
        struct POLY_TYPE
        {
            // XBID = 5
            uint8_t xblockRef; // Refers to an entry in the XBID=13 extrablock
            uint8_t carBehaviour;
        };

        // Matches number of full resolution polygons
        struct MEDIAN_BLOCK
        {
            // XBID = 6
            uint8_t refPoly[8];
        };

        struct LANE_BLOCK
        {
            // XBID = 9
            uint8_t vertRef;  // Inside B3D structure: 0 to nFullRes + nStickToNext
            uint8_t trackPos; // Position auint32_t track inside block (0 to 7)
            uint8_t latPos;   // Lateral position, -1 at the end
            uint8_t polyRef;  // Inside Full-res B3D structure, 0 to nFullRes
        };

        template <typename Platform>
        class ExtraObjectBlock : IRawData
        {
        public:
            ExtraObjectBlock() = default;
            explicit ExtraObjectBlock(std::ifstream &trk);
            void _SerializeOut(std::ofstream &ofstream) override;

            uint32_t recSize;
            uint16_t id;
            uint16_t nRecords;

            // Type 5
            std::vector<POLY_TYPE> polyTypes;

            // Type 4
            uint16_t nNeighbours;
            std::vector<uint16_t> blockNeighbours;

            // Type 8
            uint16_t nStructures;
            std::vector<typename Platform::GEOM_BLOCK> structures;
            uint16_t nStructureReferences;
            std::vector<StructureBlock> structureBlocks;

            uint16_t nUnknownVerts;
            std::vector<typename Platform::VERT> unknownVerts;

            std::vector<MEDIAN_BLOCK> medianData;
            uint16_t nVroad;
            std::vector<typename Platform::VROAD> vroadData; // Reference using XBID 5
            uint16_t nLanes;
            std::vector<LANE_BLOCK> laneData;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
        };

    } // namespace NFS2
} // namespace LibOpenNFS
