#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <cstdint>

#include "../../Util/Utils.h"

struct IntPoint
{
    int32_t x, z, y;
};

struct PolygonData
{
    uint16_t vertex[4];
    uint16_t texture;
    uint16_t hs_texflags;   // only used in road lane polygonblock ?
    unsigned char flags;    // 00 normally, 20 at end of row, 10 two-sided (HS  // used for animated textures //AnimInfo (Length : Period AS LSB 3:HSB 5))
    unsigned char unknown2; // F9
};

class RawData
{
public:
    virtual bool _SerializeIn(std::ifstream &frd) = 0;
    virtual void SerializeOut(std::ofstream &frd) = 0;
};
