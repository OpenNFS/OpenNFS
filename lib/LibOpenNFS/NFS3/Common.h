#pragma once

#include <cstdint>
#include "glm/glm.hpp"

struct PolygonData {
    uint16_t vertex[4];
    uint16_t textureId;
    uint16_t hs_texflags;   // only used in road lane polygonblock ?
    unsigned char flags;    // 00 normally, 20 at end of row, 10 two-sided (HS  // used for animated textures //AnimInfo (Length : Period AS LSB 3:HSB 5))
    unsigned char unknown2; // F9
};

struct AnimData {
    glm::ivec3 pt;
    int16_t od1, od2, od3, od4;
};