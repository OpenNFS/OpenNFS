#pragma once

#include <array>

#include "../../Common/IRawData.h"
#include "../Common.h"

namespace LibOpenNFS {
    namespace NFS3 {
        const uint8_t NUM_POLYGON_BLOCKS = 7;
        const uint8_t NUM_POLYOBJ_CHUNKS = 4;

        struct ObjectPolyBlock // a POLYOBJ chunk
        {
            uint32_t n1;                                // total number of polygons
            uint32_t n2;                                // total number of objects including XOBJs
            uint32_t nobj;                              // not stored in .FRD : number of type 1 objects
            std::vector<uint32_t> types;                // when 1, there is an associated object; else XOBJ
            std::vector<uint32_t> numpoly;              // size of each object (only for type 1 objects)
            std::vector<std::vector<PolygonData>> poly; // the polygons themselves
        };

        class PolyBlock : private IRawData {
        public:
            PolyBlock() = default;
            explicit PolyBlock(std::ifstream &frd, uint32_t nTrackBlockPolys);
            void _SerializeOut(std::ofstream &ofstream) override;

            uint32_t m_nTrackBlockPolys;

            // 7 blocks == low res / 0 / med. res / 0 / high res / 0 / ??central
            std::array<uint32_t, NUM_POLYGON_BLOCKS> sz{};
            std::array<uint32_t, NUM_POLYGON_BLOCKS> szdup{};
            std::array<std::vector<PolygonData>, NUM_POLYGON_BLOCKS> poly{};
            std::array<ObjectPolyBlock, NUM_POLYOBJ_CHUNKS> obj{}; // the POLYOBJ chunks
            // if not present, then all objects in the chunk are XOBJs
            // the 1st chunk is described anyway in the TRKBLOCK

        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
        };

    } // namespace NFS3
} // namespace LibOpenNFS
