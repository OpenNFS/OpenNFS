#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"
#include "StructureRefBlock.h"
#include "StructureBlock.h"

namespace LibOpenNFS {
    namespace NFS2 {
        // LibONFS Helper
        enum ExtraBlockID : uint8_t {
            TEXTURE_BLOCK_ID         = 2,
            NEIGHBOUR_BLOCK_ID       = 4,
            POLY_TYPE_BLOCK_ID       = 5,
            MEDIAN_BLOCK_ID          = 6,
            STRUCTURE_REF_BLOCK_A_ID = 7,
            STRUCTURE_BLOCK_ID       = 8,
            LANE_BLOCK_ID            = 9,
            UNKNOWN_BLOCK_ID_A       = 10,
            UNKNOWN_BLOCK_ID_B       = 11,
            UNKNOWN_BLOCK_ID_C       = 12,
            VROAD_BLOCK_ID           = 13,
            UNKNOWN_BLOCK_ID_D       = 14,
            COLLISION_BLOCK_ID       = 15,
            UNKNOWN_BLOCK_ID_E       = 16,
            UNKNOWN_BLOCK_ID_F       = 17,
            STRUCTURE_REF_BLOCK_B_ID = 18,
            STRUCTURE_REF_BLOCK_C_ID = 19,
            UNKNOWN_BLOCK_ID_G       = 20,
        };

        // ---- COL Specific Extra Blocks ----
        struct TEXTURE_BLOCK {
            // XBID = 2
            uint16_t texNumber; // Texture number in QFS file
            uint16_t alignmentData;
            uint8_t RGB[3];     // Luminosity
            uint8_t RGBlack[3]; // Usually black
        };

        struct COLLISION_BLOCK {
            // XBID = 15
            VERT_HIGHP trackPosition; // Position auint32_t track on a single line, either at center or side of road
            int8_t vertVec[3];        // The three vectors are mutually orthogonal, and are normalized so that
            int8_t fwdVec[3];         // each vector's norm is slightly less than 128. Each vector is coded on
            int8_t rightVec[3];       // 3 bytes : its x, z and y components are each signed 8-bit values.
            uint8_t zero;
            uint16_t blockNumber;
            uint16_t unknown; // The left and right border values indicate the two limits beyond which no car can go. This is the data used for
            // delimitation between the road and scenery
            uint16_t leftBorder; // Formula to find the coordinates of the left-most point of the road is (left-most point) = (reference point)
            // - 2.(left border).(right vector):  there is a factor of 2 between absolute
            uint16_t rightBorder; // 32-bit coordinates and the othe data in the record. Similarly, for the right-most point of the road,
            // (right-most point) = (reference point)
            // + 2.(right border).(right vector).
            uint16_t postCrashPosition; // Lateral position after respawn
            uint32_t unknown2;
        };

        // ---- TRK Specific Extra Blocks ----
        // Matches number of NP1 polygons in corresponding trackblock
        struct POLY_TYPE {
            // XBID = 5
            uint8_t xblockRef; // Refers to an entry in the XBID=13 extrablock
            uint8_t carBehaviour;
        };

        // Matches number of full resolution polygons
        struct MEDIAN_BLOCK {
            // XBID = 6
            uint8_t refPoly[8];
        };

        struct LANE_BLOCK {
            // XBID = 9
            uint8_t vertRef;  // Inside B3D structure: 0 to nFullRes + nStickToNext
            uint8_t trackPos; // Position auint32_t track inside block (0 to 7)
            uint8_t latPos;   // Lateral position, -1 at the end
            uint8_t polyRef;  // Inside Full-res B3D structure, 0 to nFullRes
        };

        struct VROAD_VEC {
            int16_t x, z, y;
        };

        struct VROAD {
            VROAD_VEC normalVec;
            VROAD_VEC forwardVec;
        };

        template <typename Platform>
        class ExtraObjectBlock : IRawData {
        public:
            ExtraObjectBlock() = default;
            explicit ExtraObjectBlock(std::ifstream &trk, NFSVersion version);
            void _SerializeOut(std::ofstream &ofstream) override;

            // ONFS attribute
            NFSVersion version;

            // Raw file data
            uint32_t recSize  = 0;
            uint16_t id       = 0;
            uint16_t nRecords = 0;

            // Type 2
            uint16_t nTextures = 0;
            std::vector<TEXTURE_BLOCK> polyToQfsTexTable;

            // Type 4
            uint16_t nNeighbours = 0;
            std::vector<int16_t> blockNeighbours;

            // Type 5
            std::vector<POLY_TYPE> polyTypes;

            // Type 6
            std::vector<MEDIAN_BLOCK> medianData;

            // Type 8
            uint16_t nStructures = 0;
            std::vector<StructureBlock<Platform>> structures;

            // Type 7, 18, 19
            uint16_t nStructureReferences = 0;
            std::vector<StructureRefBlock> structureReferences;

            // Type 9
            uint16_t nLanes = 0;
            std::vector<LANE_BLOCK> laneData;

            // Type 10?
            uint16_t nUnknownVerts = 0;
            std::vector<typename Platform::VERT> unknownVerts;

            // Type 13
            uint16_t nVroad = 0;
            std::vector<VROAD> vroadData; // Reference using XBID 5
            // Type 13 NFS2 PS1 Type (This looks hacky, but this struct doesn't match NFS3 PS1 or NFS2 PC)
            std::vector<VROAD_VEC> ps1VroadData;

            // Type 15
            uint16_t nCollisionData = 0;
            std::vector<COLLISION_BLOCK> collisionData;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
        };

    } // namespace NFS2
} // namespace LibOpenNFS
