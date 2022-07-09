#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"

namespace LibOpenNFS {
    namespace NFS2 {
        class StructureRefBlock : private IRawData {
        public:
            StructureRefBlock() = default;
            explicit StructureRefBlock(std::ifstream &trk);
            void _SerializeOut(std::ofstream &ofstream) override;

            // XBID = 7, 18
            uint16_t recSize; // If Anim (recType == 3) , recSize == 8 + 20*animation length
            uint8_t recType;
            uint8_t structureRef;
            // Fixed Type (recType == 1)
            VERT_HIGHP refCoordinates;
            // Animated Type (recType == 3)
            uint16_t animLength;                 // num of position records
            uint16_t unknown;                    // Potentially time between animation steps?
            std::vector<ANIM_POS> animationData; // Sequence of positions which animation follows

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
        };
    } // namespace NFS2
} // namespace LibOpenNFS
