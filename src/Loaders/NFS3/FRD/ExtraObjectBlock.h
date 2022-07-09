#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"

namespace LibOpenNFS {
    namespace NFS3 {
        struct ExtraObjectData {
            uint32_t crosstype; // type 4, or more rarely 3 (animated)
            uint32_t crossno;   // obj number from REFXOBJ table in TRKBLOCK
            uint32_t unknown;
            // this section only for type 4 basic objects
            glm::vec3 ptRef;
            uint32_t AnimMemory; // in HS, stores the unknown uint32_t for type 3 as well
            // this section only for type 3 animated objects
            uint16_t unknown3[9]; // 6 first are all alike; [6]==[8]=?; [7]=0
            // in HS, only 6 are used ; 6 = expected 4
            uint8_t type3, objno;            // type3==3; objno==index among all block's objects?
            uint16_t nAnimLength, AnimDelay; // JimDiabolo : The bigger the AnimDelay, that slower is the movement
            std::vector<AnimData> animData;
            // common section
            uint32_t nVertices;
            std::vector<glm::vec3> vert;
            std::vector<uint32_t> vertShading;
            uint32_t nPolygons;
            std::vector<PolygonData> polyData;
        };

        class ExtraObjectBlock : IRawData {
        public:
            ExtraObjectBlock() = default;

            explicit ExtraObjectBlock(std::ifstream &frd);

            void _SerializeOut(std::ofstream &ofstream) override;

            uint32_t nobj = 0;
            std::vector<ExtraObjectData> obj;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
        };
    } // namespace NFS3
} // namespace LibOpenNFS
