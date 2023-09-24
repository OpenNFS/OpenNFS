#pragma once

#include <array>

#include "../../Common/IRawData.h"
#include "../Common.h"

namespace LibOpenNFS {
    namespace NFS2 {
        template <typename Platform>
        class StructureBlock : private IRawData {
        public:
            StructureBlock() = default;
            explicit StructureBlock(std::ifstream &ifstream);
            void _SerializeOut(std::ofstream &ofstream) override;

            uint32_t recSize;
            uint16_t nVerts;
            uint16_t nPoly;
            std::vector<typename Platform::VERT> vertexTable;
            std::vector<typename Platform::POLYGONDATA> polygonTable;

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
        };

    } // namespace NFS2
} // namespace LibOpenNFS
