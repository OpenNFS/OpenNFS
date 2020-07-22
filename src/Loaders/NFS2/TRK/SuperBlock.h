#pragma once

#include "../../Common/IRawData.h"
#include "TrackBlock.h"

template <typename Platform>
class SuperBlock : IRawData
{
public:
    SuperBlock() = default;

    explicit SuperBlock(std::ifstream &trk);

    void _SerializeOut(std::ofstream &ofstream) override;

    uint32_t superBlockSize;
    uint32_t nBlocks;
    uint32_t padding;
    std::vector<TrackBlock<Platform>> trackBlocks;
    std::vector<uint32_t> blockOffsets;

private:
    bool _SerializeIn(std::ifstream &ifstream) override;
};
