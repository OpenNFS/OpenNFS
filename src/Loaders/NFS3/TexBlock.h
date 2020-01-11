#pragma once

#include "IFrdData.h"

class TexBlock : public IFrdData
{
public:
    TexBlock() = default;
    explicit TexBlock(std::ifstream &frd);
    void SerializeOut(std::ofstream &frd) override;

    uint16_t width, height;
    uint32_t unknown1;
    float corners[8];   // 4x planar coordinates == tiling?
    uint32_t unknown2;
    bool isLane;        // 1 if not a real texture (lane), 0 usually
    uint16_t texture;   // index in QFS file

private:
    bool _SerializeIn(std::ifstream &frd) override;
};
