#pragma once

#include "../../Common/IRawData.h"

#include "../Common.h"

struct TRKBLOCK_HEADER
{
    uint32_t blockSize;
    uint32_t blockSizeDup;
    uint16_t nExtraBlocks;
    uint16_t unknown;
    uint32_t blockSerial;
    struct VERT_HIGHP clippingRect[4];
    uint32_t extraBlockTblOffset;
    uint16_t nStickToNextVerts, nLowResVert, nMedResVert, nHighResVert;
    uint16_t nLowResPoly, nMedResPoly, nHighResPoly;
    uint16_t unknownPad[3];
};

template <typename Platform>
class TrackBlock : IRawData
{
public:
    TrackBlock() = default;

    explicit TrackBlock(std::ifstream &frd);

    void _SerializeOut(std::ofstream &ofstream) override;
    TRKBLOCK_HEADER<Platform> header;

private:
    bool _SerializeIn(std::ifstream &ifstream) override;
};
