#pragma once

#include <array>
#include "RawData.h"

struct ObjectPolyBlock  // a POLYOBJ chunk
{
    ObjectPolyBlock() = default;
    uint32_t n1;        // total number of polygons
    uint32_t n2;        // total number of objects including XOBJs
    uint32_t nobj;      // not stored in .FRD : number of type 1 objects
    std::vector<uint32_t> types; // when 1, there is an associated object; else XOBJ
    std::vector<uint32_t> numpoly;  // size of each object (only for type 1 objects)
    std::vector<std::vector<PolygonData>>  poly;    // the polygons themselves
};

const uint8_t NUM_POLYGON_BLOCKS = 7;
const uint8_t NUM_POLYOBJ_CHUNKS = 4;

class PolyBlock : private RawData
{
public:
    PolyBlock() = default;
    explicit PolyBlock(std::ifstream &frd, uint32_t nTrackBlockPolys);
    void SerializeOut(std::ofstream &frd) override;

    uint32_t m_nTrackBlockPolys;

    // 7 blocks == low res / 0 / med. res / 0 / high res / 0 / ??central
    uint32_t sz[NUM_POLYGON_BLOCKS]{};
    uint32_t szdup[NUM_POLYGON_BLOCKS]{};
    std::array<PolygonData, NUM_POLYGON_BLOCKS> poly{};
    std::vector<std::unique_ptr<ObjectPolyBlock>> obj; // the POLYOBJ chunks
    // if not present, then all objects in the chunk are XOBJs
    // the 1st chunk is described anyway in the TRKBLOCK

private:
    bool _SerializeIn(std::ifstream &frd) override;
};
