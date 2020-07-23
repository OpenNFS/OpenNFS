#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"

namespace LibOpenNFS
{
    namespace NFS2
    {
        // ---- COL Specific Extra Blocks ----
        struct TEXTURE_BLOCK
        {
            // XBID = 2
            uint16_t texNumber; // Texture number in QFS file
            uint16_t alignmentData;
            uint8_t RGB[3];     // Luminosity
            uint8_t RGBlack[3]; // Usually black
        };

        struct COLLISION_BLOCK
        {
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

        class ColFile : IRawData
        {
        public:
            ColFile() = default;
            static bool Load(const std::string &colPath, ColFile &colFile);
            static void Save(const std::string &colPath, ColFile &colFile);

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
            void _SerializeOut(std::ofstream &ofstream) override;
        };

    } // namespace NFS2
} // namespace LibOpenNFS
